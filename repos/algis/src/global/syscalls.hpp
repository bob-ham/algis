// syscalls.hpp
#pragma once
#include <windows.h>
#include <winternl.h>
#include <ntstatus.h>
#include <cstdint>
#include <cstring>



// i used a formatter for this and its still kind of ugly but yeah
namespace Syscall {
  // a bunch of structs/enums that are needed
  // for syscalls and other stuff (like the process class)

  typedef enum _SECTION_INHERIT {
    ViewShare = 1,
    ViewUnmap = 2
  } SECTION_INHERIT;

  typedef enum _SYSTEM_INFORMATION_CLASS {
    SystemBasicInformation = 0,
    SystemHandleInformation = 16,
    SystemProcessInformation = 5,
    SystemModuleInformation = 11,
  } SYSTEM_INFORMATION_CLASS;

  typedef struct _SYSTEM_HANDLE {
    ULONG ProcessId;
    UCHAR ObjectTypeNumber;
    UCHAR Flags;
    USHORT Handle;
    PVOID Object;
    ACCESS_MASK GrantedAccess;
  } SYSTEM_HANDLE, *PSYSTEM_HANDLE;

  typedef struct _SYSTEM_HANDLE_INFORMATION {
    ULONG HandleCount;
    SYSTEM_HANDLE Handles[1];
  } SYSTEM_HANDLE_INFORMATION, *PSYSTEM_HANDLE_INFORMATION;

  typedef struct _CLIENT_ID {
    HANDLE UniqueProcess;
    HANDLE UniqueThread;
  } CLIENT_ID, *PCLIENT_ID;

  typedef struct _OBJECT_ATTRIBUTES {
    ULONG Length;
    HANDLE RootDirectory;
    PUNICODE_STRING ObjectName;
    ULONG Attributes;
    PVOID SecurityDescriptor;
    PVOID SecurityQualityOfService;
  } OBJECT_ATTRIBUTES, *POBJECT_ATTRIBUTES;

  // forward decl
  template <typename Fn, typename... Args>
  inline NTSTATUS _indirect(std::uint32_t ssn, Args... args);

  struct SEntry {
    std::uint32_t ssn = 0;
    bool resolved = false;
  }; // struct SEntry

  namespace SSN {
    static SEntry NtAllocateVirtualMemory;
    static SEntry NtFreeVirtualMemory;
    static SEntry NtProtectVirtualMemory;
    static SEntry NtWriteVirtualMemory;
    static SEntry NtReadVirtualMemory;
    static SEntry NtQueryVirtualMemory;
    static SEntry NtSuspendThread;
    static SEntry NtResumeThread;
    static SEntry NtGetContextThread;
    static SEntry NtSetContextThread;
    static SEntry NtQuerySystemInformation;
    static SEntry NtQueryInformationProcess;
    static SEntry NtDuplicateObject;
    static SEntry NtQueryObject;
    static SEntry NtUnmapViewOfSection;
    static SEntry NtUnmapViewOfSectionEx;
    static SEntry NtClose;
    static SEntry NtOpenProcess;
    static SEntry NtCreateSection;
    static SEntry NtMapViewOfSection;
  } // namespace SSN

  namespace internal_ssn {
    static void* _gadget = nullptr;
    static std::uint8_t* _stub_page = nullptr;
    static std::uint32_t _stub_offset = 0;
    static bool _ready = false;

    static const std::uint32_t STUB_SIZE = 22;

    // check if bytes look like a clean nt stub
    inline bool IsStub(std::uint8_t* ptr) {
      return ptr[0] == 0x4C && ptr[1] == 0x8B && ptr[2] == 0xD1 && ptr[3] == 0xB8;
    }

    inline std::uint32_t ExtractSSN(std::uint8_t* ptr) {
      return *reinterpret_cast<std::uint32_t*>(ptr + 4);
    }

    //< halo gates resolver
    inline std::uint32_t ResolveSSN(const char* name) {
      HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
      if (!ntdll)
        return 0;

      std::uint8_t* base = reinterpret_cast<uint8_t*>(ntdll);

      IMAGE_DOS_HEADER* dos = reinterpret_cast<IMAGE_DOS_HEADER*>(base);
      IMAGE_NT_HEADERS* nt =
        reinterpret_cast<IMAGE_NT_HEADERS*>(base + dos->e_lfanew);
      IMAGE_EXPORT_DIRECTORY* exp = reinterpret_cast<IMAGE_EXPORT_DIRECTORY*>(
        base + nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT]
                 .VirtualAddress);

      DWORD* names = reinterpret_cast<DWORD*>(base + exp->AddressOfNames);
      DWORD* funcs = reinterpret_cast<DWORD*>(base + exp->AddressOfFunctions);
      WORD* ords = reinterpret_cast<WORD*>(base + exp->AddressOfNameOrdinals);

      std::int32_t target_idx = -1;
      for (DWORD i = 0; i < exp->NumberOfNames; i++) {
        if (strcmp(reinterpret_cast<const char*>(base + names[i]), name) == 0) {
          target_idx = static_cast<std::int32_t>(i);
          break;
        }
      }

      if (target_idx < 0)
        return 0;

      std::uint8_t* target_fn = base + funcs[ords[(DWORD)target_idx]];

      // clean stub; read directly
      if (IsStub(target_fn))
        return static_cast<std::uint32_t>(ExtractSSN(target_fn));

      // halos gate; walk neighbors up and down
      for (DWORD delta = 1; delta < 32; delta++) {
        if (target_idx - (int32_t)delta >= 0) {
          DWORD above_rva = funcs[ords[target_idx - (int32_t)delta]];
          uint8_t* above = base + above_rva;
          if (IsStub(above))
            return static_cast<uint32_t>(ExtractSSN(above) + delta);
        }
        if (target_idx + (int32_t)delta < (int32_t)exp->NumberOfNames) {
          DWORD below_rva = funcs[ords[target_idx + (int32_t)delta]];
          uint8_t* below = base + below_rva;
          if (IsStub(below))
            return static_cast<uint32_t>(ExtractSSN(below) - delta);
        }
      }
      return 0;
    }

    // find syscall; ret gadget
    inline void* FindGadget() {
      HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
      if (!ntdll)
        return nullptr;

      std::uint8_t* base = reinterpret_cast<std::uint8_t*>(ntdll);
      IMAGE_DOS_HEADER* dos = reinterpret_cast<IMAGE_DOS_HEADER*>(base);
      IMAGE_NT_HEADERS* nt =
        reinterpret_cast<IMAGE_NT_HEADERS*>(base + dos->e_lfanew);

      IMAGE_SECTION_HEADER* sec = IMAGE_FIRST_SECTION(nt);
      for (WORD i = 0; i < nt->FileHeader.NumberOfSections; i++, sec++) {
        if (std::memcmp(sec->Name, ".text", 5) != 0)
          continue;

        std::uint8_t* start = base + sec->VirtualAddress;
        std::uint8_t* end = start + sec->SizeOfRawData - 2;

        for (uint8_t* p = start; p < end; p++) {
          if (p[0] == 0x0F && p[1] == 0x05 && p[2] == 0xC3)
            return p;
        }
      }
      return nullptr;
    }

    // get or build stub
    inline std::uint8_t* GetOrBuildStub(std::uint32_t ssn) {
      if (!_stub_page) {
        _stub_page = reinterpret_cast<uint8_t*>(VirtualAlloc(
          nullptr, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));
        if (!_stub_page)
          return nullptr;
      }

      // linear scan for existing stub
      for (uint32_t i = 0; i < _stub_offset; i += STUB_SIZE) {
        uint8_t* s = _stub_page + i;
        if (*reinterpret_cast<uint32_t*>(s + 1) == ssn)
          return s;
      }

      // build new stub
      uint8_t* stub = _stub_page + _stub_offset;
      _stub_offset += STUB_SIZE;

      // mov eax, ssn
      stub[0] = 0xB8;
      *reinterpret_cast<uint32_t*>(stub + 1) = ssn;

      // mov r10, rcx
      stub[5] = 0x4C;
      stub[6] = 0x8B;
      stub[7] = 0xD1;

      // jmp qword ptr [rip+0]
      stub[8] = 0xFF;
      stub[9] = 0x25;
      stub[10] = 0x00;
      stub[11] = 0x00;
      stub[12] = 0x00;
      stub[13] = 0x00;

      // gadget pointer immediately after the jmp instruction
      *reinterpret_cast<void**>(stub + 14) = _gadget;

      return stub;
    }

  } // namespace internal_ssn

  // indirect dispatch
  template <typename Fn, typename... Args>
  inline NTSTATUS _indirect(std::uint32_t ssn, Args... args) {
    std::uint8_t* stub = internal_ssn::GetOrBuildStub(ssn);
    if (!stub)
      return STATUS_UNSUCCESSFUL;
    return reinterpret_cast<Fn>(stub)(args...);
  }

  // init, call before using wrappers
  inline bool Init() {
    if (internal_ssn::_ready)
      return true;
    internal_ssn::_gadget = internal_ssn::FindGadget();
    if (!internal_ssn::_gadget)
      return false;

    auto resolve = [](SEntry& e, const char* name) {
      e.ssn = internal_ssn::ResolveSSN(name);
      e.resolved = (e.ssn != 0);
    };

    resolve(SSN::NtAllocateVirtualMemory, "NtAllocateVirtualMemory");
    resolve(SSN::NtFreeVirtualMemory, "NtFreeVirtualMemory");
    resolve(SSN::NtProtectVirtualMemory, "NtProtectVirtualMemory");
    resolve(SSN::NtWriteVirtualMemory, "NtWriteVirtualMemory");
    resolve(SSN::NtReadVirtualMemory, "NtReadVirtualMemory");
    resolve(SSN::NtQueryVirtualMemory, "NtQueryVirtualMemory");
    resolve(SSN::NtSuspendThread, "NtSuspendThread");
    resolve(SSN::NtResumeThread, "NtResumeThread");
    resolve(SSN::NtGetContextThread, "NtGetContextThread");
    resolve(SSN::NtSetContextThread, "NtSetContextThread");
    resolve(SSN::NtQuerySystemInformation, "NtQuerySystemInformation");
    resolve(SSN::NtQueryInformationProcess, "NtQueryInformationProcess");
    resolve(SSN::NtDuplicateObject, "NtDuplicateObject");
    resolve(SSN::NtQueryObject, "NtQueryObject");
    resolve(SSN::NtUnmapViewOfSection, "NtUnmapViewOfSection");
    resolve(SSN::NtUnmapViewOfSectionEx, "NtUnmapViewOfSectionEx");
    resolve(SSN::NtClose, "NtClose");
    resolve(SSN::NtOpenProcess, "NtOpenProcess");
    resolve(SSN::NtCreateSection, "NtCreateSection");
    resolve(SSN::NtMapViewOfSection, "NtMapViewOfSection");

    internal_ssn::_ready = true;
    return true;
  }

  // syscall wrappers
  inline NTSTATUS AllocateVirtualMemory(HANDLE ProcessHandle, PVOID* BaseAddress, ULONG_PTR ZeroBits, PSIZE_T RegionSize, ULONG AllocationType, ULONG Protect) {
    using Fn =
      NTSTATUS(NTAPI*)(HANDLE, PVOID*, ULONG_PTR, PSIZE_T, ULONG, ULONG);
    return _indirect<Fn>(SSN::NtAllocateVirtualMemory.ssn, ProcessHandle, BaseAddress, ZeroBits, RegionSize, AllocationType, Protect);
  }

  inline NTSTATUS FreeVirtualMemory(HANDLE ProcessHandle, PVOID* BaseAddress, PSIZE_T RegionSize, ULONG FreeType) {
    using Fn = NTSTATUS(NTAPI*)(HANDLE, PVOID*, PSIZE_T, ULONG);
    return _indirect<Fn>(SSN::NtFreeVirtualMemory.ssn, ProcessHandle, BaseAddress, RegionSize, FreeType);
  }

  inline NTSTATUS ProtectVirtualMemory(HANDLE ProcessHandle, PVOID* BaseAddress, PSIZE_T RegionSize, ULONG NewProtect, PULONG OldProtect) {
    using Fn = NTSTATUS(NTAPI*)(HANDLE, PVOID*, PSIZE_T, ULONG, PULONG);
    return _indirect<Fn>(SSN::NtProtectVirtualMemory.ssn, ProcessHandle, BaseAddress, RegionSize, NewProtect, OldProtect);
  }
  inline NTSTATUS CreateSection(PHANDLE SectionHandle, ACCESS_MASK DesiredAccess, POBJECT_ATTRIBUTES ObjectAttributes, PLARGE_INTEGER MaximumSize, ULONG SectionPageProtection, ULONG AllocationAttributes, HANDLE FileHandle) {
    using Fn = NTSTATUS(NTAPI*)(PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES, PLARGE_INTEGER, ULONG, ULONG, HANDLE);
    return _indirect<Fn>(SSN::NtCreateSection.ssn, SectionHandle, DesiredAccess, ObjectAttributes, MaximumSize, SectionPageProtection, AllocationAttributes, FileHandle);
  }

  inline NTSTATUS MapViewOfSection(HANDLE SectionHandle, HANDLE ProcessHandle, PVOID* BaseAddress, ULONG_PTR ZeroBits, SIZE_T CommitSize, PLARGE_INTEGER SectionOffset, PSIZE_T ViewSize, DWORD InheritDisposition, ULONG AllocationType, ULONG Win32Protect) {
    using Fn = NTSTATUS(NTAPI*)(HANDLE, HANDLE, PVOID*, ULONG_PTR, SIZE_T, PLARGE_INTEGER, PSIZE_T, DWORD, ULONG, ULONG);
    return _indirect<Fn>(SSN::NtMapViewOfSection.ssn, SectionHandle, ProcessHandle, BaseAddress, ZeroBits, CommitSize, SectionOffset, ViewSize, InheritDisposition, AllocationType, Win32Protect);
  }
  inline NTSTATUS Close(HANDLE ProcessHandle) {
    using Fn = NTSTATUS(NTAPI*)(HANDLE);
    return _indirect<Fn>(SSN::NtClose.ssn, ProcessHandle);
  }
  inline NTSTATUS OpenProcess(PHANDLE ProcessHandle, ACCESS_MASK DesiredAccess, POBJECT_ATTRIBUTES ObjectAttributes, PCLIENT_ID ClientId) {
    using Fn =
      NTSTATUS(NTAPI*)(PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES, PCLIENT_ID);
    return _indirect<Fn>(SSN::NtOpenProcess.ssn, ProcessHandle, DesiredAccess, ObjectAttributes, ClientId);
  }
  inline NTSTATUS WriteVirtualMemory(HANDLE ProcessHandle, PVOID BaseAddress, PVOID Buffer, SIZE_T NumberOfBytesToWrite, PSIZE_T NumberOfBytesWritten) {
    using Fn = NTSTATUS(NTAPI*)(HANDLE, PVOID, PVOID, SIZE_T, PSIZE_T);
    return _indirect<Fn>(SSN::NtWriteVirtualMemory.ssn, ProcessHandle, BaseAddress, Buffer, NumberOfBytesToWrite, NumberOfBytesWritten);
  }

  inline NTSTATUS ReadVirtualMemory(HANDLE ProcessHandle, PVOID BaseAddress, PVOID Buffer, SIZE_T NumberOfBytesToRead, PSIZE_T NumberOfBytesRead) {
    using Fn = NTSTATUS(NTAPI*)(HANDLE, PVOID, PVOID, SIZE_T, PSIZE_T);
    return _indirect<Fn>(SSN::NtReadVirtualMemory.ssn, ProcessHandle, BaseAddress, Buffer, NumberOfBytesToRead, NumberOfBytesRead);
  }

  inline NTSTATUS QueryVirtualMemory(HANDLE ProcessHandle, PVOID BaseAddress, DWORD MemoryInformationClass, PVOID MemoryInformation, SIZE_T MemoryInformationLength, PSIZE_T ReturnLength) {
    using Fn = NTSTATUS(NTAPI*)(HANDLE, PVOID, DWORD, PVOID, SIZE_T, PSIZE_T);
    return _indirect<Fn>(SSN::NtQueryVirtualMemory.ssn, ProcessHandle, BaseAddress, MemoryInformationClass, MemoryInformation, MemoryInformationLength, ReturnLength);
  }

  inline NTSTATUS SuspendThread(HANDLE ThreadHandle,
    PULONG PreviousSuspendCount = nullptr) {
    using Fn = NTSTATUS(NTAPI*)(HANDLE, PULONG);
    return _indirect<Fn>(SSN::NtSuspendThread.ssn, ThreadHandle, PreviousSuspendCount);
  }

  inline NTSTATUS ResumeThread(HANDLE ThreadHandle,
    PULONG PreviousSuspendCount = nullptr) {
    using Fn = NTSTATUS(NTAPI*)(HANDLE, PULONG);
    return _indirect<Fn>(SSN::NtResumeThread.ssn, ThreadHandle, PreviousSuspendCount);
  }

  inline NTSTATUS GetContextThread(HANDLE ThreadHandle, PCONTEXT ThreadContext) {
    using Fn = NTSTATUS(NTAPI*)(HANDLE, PCONTEXT);
    return _indirect<Fn>(SSN::NtGetContextThread.ssn, ThreadHandle, ThreadContext);
  }

  inline NTSTATUS SetContextThread(HANDLE ThreadHandle, PCONTEXT ThreadContext) {
    using Fn = NTSTATUS(NTAPI*)(HANDLE, PCONTEXT);
    return _indirect<Fn>(SSN::NtSetContextThread.ssn, ThreadHandle, ThreadContext);
  }

  inline NTSTATUS
  QuerySystemInformation(SYSTEM_INFORMATION_CLASS SystemInformationClass,
    PVOID SystemInformation,
    ULONG SystemInformationLength,
    PULONG ReturnLength) {
    using Fn = NTSTATUS(NTAPI*)(SYSTEM_INFORMATION_CLASS, PVOID, ULONG, PULONG);
    return _indirect<Fn>(SSN::NtQuerySystemInformation.ssn,
      SystemInformationClass,
      SystemInformation,
      SystemInformationLength,
      ReturnLength);
  }

  inline NTSTATUS
  QueryInformationProcess(HANDLE ProcessHandle,
    PROCESSINFOCLASS ProcessInformationClass,
    PVOID ProcessInformation,
    ULONG ProcessInformationLength,
    PULONG ReturnLength) {
    using Fn = NTSTATUS(NTAPI*)(HANDLE, PROCESSINFOCLASS, PVOID, ULONG, PULONG);
    return _indirect<Fn>(SSN::NtQueryInformationProcess.ssn, ProcessHandle, ProcessInformationClass, ProcessInformation, ProcessInformationLength, ReturnLength);
  }

  inline NTSTATUS DuplicateObject(HANDLE SourceProcessHandle, HANDLE SourceHandle, HANDLE TargetProcessHandle, PHANDLE TargetHandle, ACCESS_MASK DesiredAccess, ULONG HandleAttributes, ULONG Options) {
    using Fn = NTSTATUS(NTAPI*)(HANDLE, HANDLE, HANDLE, PHANDLE, ACCESS_MASK, ULONG, ULONG);
    return _indirect<Fn>(SSN::NtDuplicateObject.ssn, SourceProcessHandle, SourceHandle, TargetProcessHandle, TargetHandle, DesiredAccess, HandleAttributes, Options);
  }

  inline NTSTATUS QueryObject(HANDLE Handle,
    OBJECT_INFORMATION_CLASS ObjectInformationClass,
    PVOID ObjectInformation,
    ULONG ObjectInformationLength,
    PULONG ReturnLength) {
    using Fn =
      NTSTATUS(NTAPI*)(HANDLE, OBJECT_INFORMATION_CLASS, PVOID, ULONG, PULONG);
    return _indirect<Fn>(SSN::NtQueryObject.ssn, Handle, ObjectInformationClass, ObjectInformation, ObjectInformationLength, ReturnLength);
  }
  inline NTSTATUS UnmapViewOfSection(HANDLE ProcessHandle, PVOID BaseAddress) {
    using Fn = NTSTATUS(NTAPI*)(HANDLE, PVOID);
    return _indirect<Fn>(SSN::NtUnmapViewOfSection.ssn, ProcessHandle, BaseAddress);
  }

  inline NTSTATUS UnmapViewOfSectionEx(HANDLE ProcessHandle, PVOID BaseAddress, ULONG Flags) {
    using Fn = NTSTATUS(NTAPI*)(HANDLE, PVOID, ULONG);
    return _indirect<Fn>(SSN::NtUnmapViewOfSectionEx.ssn, ProcessHandle, BaseAddress, Flags);
  }

} // namespace Syscall