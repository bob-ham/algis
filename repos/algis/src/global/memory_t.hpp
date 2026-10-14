// memory_t.hpp
#pragma once
#include "../external/hooker/process.hpp"
#include <TlHelp32.h>
#include <memory>

namespace {
  static bool safe_read(uintptr_t address, void* buffer, size_t size) {
    if (!address || address < 0x10000 || address > 0x7FFFFFFFFFFF)
      return false;
    if (IsBadReadPtr(reinterpret_cast<void*>(address), size))
      return false;
    __try {
      CopyMemory(buffer, reinterpret_cast<void*>(address), size);
      return true;
    } __except (EXCEPTION_EXECUTE_FAULT) {
      return false;
    }
  }

  static bool safe_write(uintptr_t address, const void* buffer, size_t size) {
    if (!address || address < 0x10000 || address > 0x7FFFFFFFFFFF)
      return false;
    if (IsBadWritePtr(reinterpret_cast<void*>(address), size))
      return false;
    __try {
      CopyMemory(reinterpret_cast<void*>(address), buffer, size);
      return true;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
      return false;
    }
  }
} // namespace

class memory_t final : public Process {
public:
  explicit memory_t(std::string process_name)
      : Process(std::move(process_name)) {}
  ~memory_t() = default;

  static constexpr bool is_valid(std::uint64_t address) {
    return address >= 0x10000 && address <= 0x7FFFFFFEFFFF;
  }

  std::uint64_t find_module_address(const std::string& module_name) {
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, get_pid());
    if (snap == INVALID_HANDLE_VALUE)
      return 0;

    MODULEENTRY32 entry{};
    entry.dwSize = sizeof(MODULEENTRY32);

    if (Module32First(snap, &entry)) {
      do {
        if (module_name == entry.szModule) {
          CloseHandle(snap);
          return reinterpret_cast<std::uint64_t>(entry.modBaseAddr);
        }
      } while (Module32Next(snap, &entry));
    }

    CloseHandle(snap);
    return 0;
  }

  // to get handle, use get_handle()

  template <typename T>
  T read(std::uint64_t address) {
    T buffer{};
    SIZE_T bytes_read = 0;
    read_memr(reinterpret_cast<PVOID>(address), &buffer, sizeof(T), &bytes_read);
    return buffer;
  }

  template <typename T>
  void write(std::uint64_t address, T value) {
    SIZE_T bytes_written = 0;
    write_memr(reinterpret_cast<PVOID>(address), &value, sizeof(T), &bytes_written);
  }
};

class internal_memory_t final {
public:
  inline std::uintptr_t get_module_base() {
    return (std::uintptr_t)GetModuleHandleA(NULL);
  }

  template <typename T>
  inline T read(std::uintptr_t address) {
    T value{};
    safe_read(address, &value, sizeof(T));
    return value;
  }

  template <typename T>
  inline bool write(std::uintptr_t address, T value) {
    return safe_write(address, &value, sizeof(T));
  }
};

inline std::unique_ptr<memory_t>& driver(const std::string& process_name = "") {
  static std::unique_ptr<memory_t> instance;
  if (!instance && !process_name.empty())
    instance = std::make_unique<memory_t>(process_name);
  return instance;
}

inline std::unique_ptr<internal_memory_t>& i_driver() {
  static std::unique_ptr<internal_memory_t> instance;
  if (!instance) {
    instance = std::make_unique<internal_memory_t>();
  }
  return instance;
}