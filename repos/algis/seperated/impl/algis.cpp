// algis.cpp; defs file
#include "../algis.h"
#include "../global/ipc.hpp"

#if defined(ALGIS_EXTERNAL)
#include "../external/hooker/process.hpp"
#include "../external/hooker/mmap.hpp"
#include "../external/hooker/hook.hpp"
#endif

#if !defined(ALGIS_LEAN)
#include "../global/network.hpp"
#endif

#if defined (ALGIS_INTERNAL)
#include "../internal/ipc_log.hpp"
#include "../internal/dispatch.hpp"
#endif

// function defs

#if defined(ALGIS_EXTERNAL)
ALGIS_API algis::hook_init(const char* process_name) {
  auto& lib = GetHookLib();
  if (lib.process)
    return AL_INTERNAL_UNKNOWN_SYSTEM_ERROR;

  lib.process = new Process(std::string(process_name));

  if (lib.process->get_pid() == 0) {
    delete lib.process;
    lib.process = nullptr;
    return AL_PROCESS_NOT_FOUND;
  }
  return ALGIS_OK;
}

ALGIS_API algis::hook_uninit(VOID) {
  auto& lib = GetHookLib();
  delete lib.process;
  lib.process = nullptr;
  return ALGIS_OK;
}

ALGIS_API algis::map(const wchar_t* dll_path, uintptr_t* o_entrypoint) {
  auto& lib = GetHookLib();
  if (!lib.process)
    return AL_INTERNAL_UNKNOWN_SYSTEM_ERROR;

  mapper mmap(*lib.process);
  lib.last_map_result = mmap.map(dll_path);

  if (!lib.last_map_result.success) {
    return AL_ERR_MAPPING_FAILED;
  }

  if (o_entrypoint) {
    *o_entrypoint = lib.last_map_result.entry_point;
  }
  return ALGIS_OK;
}

ALGIS_API algis::create_hook(void* jmp_instruction, void* new_target, HOOK* o_hook) {
  auto& lib = GetHookLib();
  if (!lib.process)
    return AL_INTERNAL_UNKNOWN_SYSTEM_ERROR;
  if (lib.registry.find(jmp_instruction) != lib.registry.end())
    return AL_HOOK_FAILED;

  DWORD target_pid = lib.process->get_pid();
  lib.process->suspend_all_threads(target_pid, true);

  Hook* h = new Hook();

  bool success = h->hook(*lib.process, jmp_instruction, new_target);

  lib.process->suspend_all_threads(target_pid, false);

  if (!success) {
    delete h;
    return AL_HOOK_FAILED;
  }

  lib.registry[jmp_instruction] = h;
  *o_hook = (void*)h;

  return ALGIS_OK;
}

ALGIS_API algis::destroy_hook(HOOK hook) {
  auto& lib = GetHookLib();
  if (!lib.process)
    return AL_INTERNAL_UNKNOWN_SYSTEM_ERROR;

  DWORD targetPID = lib.process->get_pid();
  lib.process->suspend_all_threads(targetPID, true);

  if (hook != DESTROY_HOOK_ALL) {
    Hook* h = (Hook*)hook;
    h->unhook(*lib.process);

    for (auto it = lib.registry.begin(); it != lib.registry.end(); ++it) {
      if (it->second == h) {
        lib.registry.erase(it);
        break;
      }
    }
    delete h;
  } else {
    for (auto& pair : lib.registry) {
      Hook* hookObj = pair.second;
      hookObj->unhook(*lib.process);
      delete hookObj;
    }
    lib.registry.clear();
  }

  lib.process->suspend_all_threads(targetPID, false);
  return ALGIS_OK;
}

#endif

// global code

// ipc section
ALGIS_API algis::create_pipe(LPCWSTR pipe_name, DWORD buffer_size, HANDLE* o_handle) {
  return _create_pipe_(pipe_name, buffer_size, o_handle);
}

ALGIS_API algis::open_pipe(LPCWSTR pipe_name, DWORD ms, HANDLE* o_handle) {
  return _open_pipe_(pipe_name, ms, o_handle);
}

ALGIS_API algis::destroy_pipe(HANDLE* handle) {
  return _destroy_pipe_(handle);
}

ALGIS_API algis::create_smem(LPCWSTR mem_name, int buffer_size, HANDLE* o_handle, void** o_buffer) {
  return _create_smem_(mem_name, buffer_size, o_handle, o_buffer);
}

ALGIS_API algis::open_smem(LPCWSTR mem_name, int buffer_size, HANDLE* o_handle, void** o_buffer) {
  return _open_smem_(mem_name, buffer_size, o_handle, o_buffer);
}

ALGIS_API algis::destroy_smem(HANDLE o_handle, void* buffer) {
  return _destroy_smem_(o_handle, buffer);
}

ALGIS_API xsend(int transport_type, LPCWSTR ipc_name, const void* data, size_t data_size, bool seperate_thread = false) {
  if (!ipc_name || !data || data_size == 0)
    return AL_INVALID_PARAMETER;

  if (seperate_thread) {
    std::vector<BYTE> data_copy(static_cast<const BYTE*>(data), static_cast<const BYTE*>(data) + data_size);
    std::wstring name_copy(ipc_name);

    std::thread([transport_type, name_copy, data_copy]() {
      _xsend_(transport_type, name_copy.c_str(), data_copy.data(), data_copy.size());
    }).detach();
    return ALGIS_OK;
  } else {
    return _xsend_(transport_type, ipc_name, data, data_size);
  }
}

ALGIS_API xrecieve(int transport_type, LPCWSTR ipc_name, void* o_buffer, size_t buffer_size, DWORD* o_bytes_read, DWORD ms, fnAlgisCallback callback = nullptr, bool seperate_thread = false) {
  if (seperate_thread) {
    if (!callback)
      return AL_INVALID_PARAMETER;

    std::wstring name_copy(ipc_name);

    std::thread([transport_type, name_copy, ms, callback]() {
      std::vector<BYTE> local_buffer(4096);
      DWORD bytes_read = 0;

      ALGIS_STATUS status = _xrecieve_(transport_type, name_copy.c_str(), local_buffer.data(), local_buffer.size(), &bytes_read, ms);
      if (status == ALGIS_OK && bytes_read > 0) {
        callback(local_buffer.data(), bytes_read);
      }
    }).detach();
    return ALGIS_OK;
  } else {
    return _xrecieve_(transport_type, ipc_name, o_buffer, buffer_size, o_bytes_read, ms);
  }
}

#if defined(ALGIS_INTERNAL)
ALGIS_API ipc_log(LPCWSTR ipc_name, std::uint8_t level, const char* msg) {
  return _ipc_log_(ipc_name, level, msg);
}

#endif

#if !defined(ALGIS_LEAN)
ALGIS_API algis::fetch(const std::string &url, const std::string &save_path) {
  return _fetch_(url, save_path);
}

std::uintptr_t algis::get(const std::string &name) {
  return _get_(name);
}
#endif