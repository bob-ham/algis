// process.cpp
#include <iostream>
#include "process.hpp"
#include "../../global/syscalls.hpp"
#include <TlHelp32.h>

// process class defs
HANDLE Process::get_handle() { return this->handle; }
DWORD Process::get_pid() { return this->pid; }

DWORD Process::fetch_pid(std::string name) {
  HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (snapshot == INVALID_HANDLE_VALUE)
    return 0;

  PROCESSENTRY32W PE;
  PE.dwSize = sizeof(PROCESSENTRY32W);
  std::wstring wname(name.begin(), name.end());

  if (Process32FirstW(snapshot, &PE)) {
    do {
      if (wcscmp(PE.szExeFile, wname.c_str()) == 0) {
        CloseHandle(snapshot);
        return PE.th32ProcessID;
      }
    } while (Process32NextW(snapshot, &PE));
  }
  CloseHandle(snapshot);
  return 0;
}

inline HANDLE Process::hijack_handle(DWORD targetPid) {
  HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
  typedef NTSTATUS(NTAPI * _RtlAdjustPrivilege)(ULONG, BOOLEAN, BOOLEAN, PBOOLEAN);
  auto RtlAdjustPrivilege = reinterpret_cast<_RtlAdjustPrivilege>(
    GetProcAddress(ntdll, "RtlAdjustPrivilege"));

  BOOLEAN oldPriv;
  if (RtlAdjustPrivilege)
    RtlAdjustPrivilege(20, TRUE, FALSE, &oldPriv);

  NTSTATUS status = 0;
  SIZE_T size = 0x10000;
  PVOID buf = nullptr;

  status = Syscall::AllocateVirtualMemory(
    GetCurrentProcess(), &buf, 0, &size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

  if (!NT_SUCCESS(status) || !buf)
    return nullptr;

  while ((status = Syscall::QuerySystemInformation(
            Syscall::SystemHandleInformation,
            buf,
            static_cast<ULONG>(size),
            reinterpret_cast<PULONG>(&size))) == STATUS_INFO_LENGTH_MISMATCH) {
    SIZE_T zero = 0;
    Syscall::FreeVirtualMemory(GetCurrentProcess(), &buf, &zero, MEM_RELEASE);
    buf = nullptr;
    size *= 2;

    status = Syscall::AllocateVirtualMemory(
      GetCurrentProcess(), &buf, 0, &size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

    if (!NT_SUCCESS(status) || !buf)
      return nullptr;
  }

  if (!NT_SUCCESS(status)) {
    SIZE_T zero = 0;
    Syscall::FreeVirtualMemory(GetCurrentProcess(), &buf, &zero, MEM_RELEASE);
    return nullptr;
  }

  auto* hInfo = reinterpret_cast<Syscall::PSYSTEM_HANDLE_INFORMATION>(buf);
  HANDLE result = nullptr;

  for (ULONG i = 0; i < hInfo->HandleCount; i++) {
    Syscall::SYSTEM_HANDLE& sh = hInfo->Handles[i];
    if (!sh.Handle)
      continue;

    Syscall::OBJECT_ATTRIBUTES objAttr = {sizeof(OBJECT_ATTRIBUTES)};
    CLIENT_ID cid = {reinterpret_cast<HANDLE>((ULONG_PTR)sh.ProcessId), nullptr};
    HANDLE owner = nullptr;

    status = Syscall::OpenProcess(&owner, PROCESS_DUP_HANDLE, &objAttr, reinterpret_cast<Syscall::PCLIENT_ID>(&cid));
    if (!NT_SUCCESS(status) || !owner)
      continue;

    HANDLE dup = nullptr;
    status = Syscall::DuplicateObject(
      owner,
      reinterpret_cast<HANDLE>((ULONG_PTR)sh.Handle),
      GetCurrentProcess(),
      &dup,
      PROCESS_ALL_ACCESS,
      0,
      0);

    Syscall::Close(owner);

    if (NT_SUCCESS(status) && dup) {
      if (GetProcessId(dup) == targetPid) {
        result = dup;
        break;
      }
      Syscall::Close(dup);
    }
  }

  SIZE_T zero = 0;
  Syscall::FreeVirtualMemory(GetCurrentProcess(), &buf, &zero, MEM_RELEASE);
  return result;
}

Process::Process(std::string target_name) {
  Syscall::Init();
  this->name = target_name;
  this->pid = fetch_pid(target_name);
  this->handle = hijack_handle(this->pid);

  if (!this->handle) {
    std::cerr << "failed to hijack handle for: " << target_name << std::endl;
  }
}

Process::~Process() {
  if (handle)
    Syscall::Close(handle);
}

NTSTATUS Process::read_memr(PVOID base, PVOID buf, SIZE_T size, PSIZE_T read) {
  return Syscall::ReadVirtualMemory(this->handle, base, buf, size, read);
}

NTSTATUS Process::write_memr(PVOID base, PVOID buf, SIZE_T siz, PSIZE_T written) {
  return Syscall::WriteVirtualMemory(this->handle, base, buf, siz, written);
}

NTSTATUS Process::change_protection(PVOID base, SIZE_T size, ULONG prot, PULONG oldProt) {
  PVOID address_to_protect = base;
  SIZE_T region_size = size;
  return Syscall::ProtectVirtualMemory(this->handle, &address_to_protect, &region_size, prot, oldProt);
}

PVOID Process::allocate_mem(SIZE_T size, ULONG type, ULONG prot) {
  PVOID base = nullptr;
  SIZE_T region_size = size;
  if (true) {
    Syscall::AllocateVirtualMemory(this->handle, &base, 0, &region_size, type, prot);
  }
  return base;
}

void Process::suspend_all_threads(DWORD pid, bool suspend) {
  HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
  if (hSnapshot == INVALID_HANDLE_VALUE)
    return;

  THREADENTRY32 te;
  te.dwSize = sizeof(THREADENTRY32);

  if (Thread32First(hSnapshot, &te)) {
    do {
      if (te.th32OwnerProcessID == pid) {
        HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, te.th32ThreadID);
        if (hThread != NULL) {
          if (suspend)
            SuspendThread(hThread);
          else
            ResumeThread(hThread);

          CloseHandle(hThread);
        }
      }
    } while (Thread32Next(hSnapshot, &te));
  }
  CloseHandle(hSnapshot);
}