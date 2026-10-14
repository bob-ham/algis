// process.hpp
#pragma once
#include <windows.h>
#include <string>


class Process {
private:
  std::string name;
  DWORD pid;
  HANDLE handle;

  HANDLE hijack_handle(DWORD target_pid);
  DWORD fetch_pid(std::string name);

public:
  Process(std::string name);
  ~Process();

  Process(const Process& other) = delete;
  Process& operator=(const Process& other) = delete;

  inline Process(Process&& other) noexcept
      : name(std::move(other.name)), pid(other.pid), handle(other.handle) {
    other.pid = 0;
    other.handle = nullptr;
  }

  inline Process& operator=(Process&& other) noexcept {
    if (this != &other) {
      if (handle)
        CloseHandle(handle);
      name = std::move(other.name);
      pid = other.pid;
      handle = other.handle;
      other.pid = 0;
      other.handle = nullptr;
    }
    return *this;
  }

  HANDLE get_handle();
  DWORD get_pid();

  // change; read_memr -> read_memory_raw
  // this was added as im going to make a template for read
  NTSTATUS read_memr(PVOID base, PVOID buf, SIZE_T size, PSIZE_T read);
  // change; write_memr -> write_memory_raw
  // same reason as read_memr
  NTSTATUS write_memr(PVOID base, PVOID buf, SIZE_T siz, PSIZE_T written);
  NTSTATUS change_protection(PVOID base, SIZE_T size, ULONG prot, PULONG oldProt);
  PVOID allocate_mem(SIZE_T size, ULONG type, ULONG prot);

  void suspend_all_threads(DWORD pid, bool suspend);
};
