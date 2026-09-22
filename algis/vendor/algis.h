/*
* All credits go to freakster (@_exthk on Instagram and TikTok)
*/
#ifndef ALGIS_H
#define ALGIS_H

// OS checks
#if defined(_WIN32) || defined(WIN32) || defined(_WIN64)
#define _OS_WINDOWS 1
#elif defined(__linux__) || defined(__linux)
#define _OS_LINUX 1
#endif


#if defined(_OS_WINDOWS)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <ntstatus.h>
#include <winternl.h>
#include <intrin.h>
#include <tlhelp32.h>
#elif defined(_OS_LINUX)
#include <unistd.h>
#include <dlfcn.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <elf.h>
#include <link.h>
#endif
#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <cstddef>
#include <map>
#include <mutex>
#include <cerrno>
#include <functional>
#include <utility>
#include <fstream>
#include <iomanip>
#include <cstdarg>

//        __     _   
//  ___ _/ /__ _(_)__
// / _ `/ / _ `/ (_-<
// \_,_/_/\_, /_/___/
//       /___/          

// ALGIS_H - A single header file, with the capabilities of IAT hooking, VMT hooking, IPC helper, logging, networking, and even more to come in the future!
// linux support is lacking currently, i want to finish the windows code and then linux

// important constants/macros.
// #define ALGIS_INTERNAL //< user defined; are you internal (inside the program?) or external (a seperate .exe running outside?)
// #define ALGIS_EXTERNAL //< user defined; are you internal (inside the program?) or external (a seperate .exe running outside?)
// #define ALGIS_LEAN //< cuts the networking part of the lib (~1-2,000 lines).

// constants defs.
#ifndef XPIPE
#define XPIPE 1 //< used in the function XSend and XRecieve, important
#endif

#ifndef XSMEM
#define XSMEM 2 //< used in the function XSend and XRecieve, important
#endif

#ifndef H_DESTROY
#define H_DESTROY nullptr //< used to destroy every IAT/VMT hook
#endif

#ifndef IPC_THREAD_CREATE
#define IPC_THREAD_CREATE true //< used to create an IPC in a seperate thread
#endif

#ifndef IPC_MAGIC
#define IPC_MAGIC "algis_ipc" //< used for auth between IPC's
#endif


typedef void* A_HOOK; //< shortname for void* basically
typedef void(*pFnAlgisCallback)(void* data, size_t size);

#if defined(_OS_WINDOWS)
#ifndef NT_SUCCESS
#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)
#endif


#ifndef NTSTATUS
typedef LONG NTSTATUS;
#endif

#define __attr_pack__  __pragma(pack(push, 1))
#define __attr_pop__   __pragma(pack(pop))
#elif defined(_OS_LINUX)
#define __attr_pack__  _Pragma("pack(push, 1)")
#define __attr_pop__   _Pragma("pack(pop)")
#endif


#if defined(ALGIS_INTERNAL) && defined(ALGIS_EXTERNAL)
#error "cannot define both ALGIS_INTERNAL and ALGIS_EXTERNAL"
#endif

#if !defined(ALGIS_INTERNAL) && !defined(ALGIS_EXTERNAL)
#define ALGIS_EXTERNAL
#endif

#if !(defined _M_IX86) && !(defined _M_X64) && !(defined __i386__) && !(defined __x86_64__)
#error "X86-64 are the only supported types."
#endif

class f_logger {
private:
    std::ofstream m_file;
    std::mutex m_mutex;

    f_logger() = default;
    ~f_logger() {
        if (m_file.is_open()) {
            m_file.close();
        }
    }

    f_logger(const f_logger&) = delete;
    f_logger& operator=(const f_logger&) = delete;

public:
    static f_logger& g_instance() {
        static f_logger instance;
        return instance;
    }

    bool init(const std::string& filename) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_file.open(filename, std::ios::out | std::ios::app);
        return m_file.is_open();
    }

    template <typename ... Args>
    void log(const char* level, const char* format, Args... args) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_file.is_open()) return;

        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);

        struct tm timeinfo;
#if defined(_WIN32)
        localtime_s(&timeinfo, &in_time_t);
#else
        localtime_r(&in_time_t, &timeinfo);
#endif

        char timeBuf[32];
        std::strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S", &timeinfo);

        char msgBuf[1024];
        std::snprintf(msgBuf, sizeof(msgBuf), format, args...);

        m_file << "[" << timeBuf << "] [" << level << "] " << msgBuf << '\n';
        m_file.flush();
    }
};

/// TO USE LOG_INIT, DO LOG_INIT("log.log") OR LOG_INIT("C:\\Users\\public\\runtime.log");
#define LOG_INIT(filename) f_logger::g_instance().init(filename)
#define ILOG(fmt, ...)     f_logger::g_instance().log("INFO", fmt, ##__VA_ARGS__)
#define WLOG(fmt, ...)     f_logger::g_instance().log("WARN", fmt, ##__VA_ARGS__)
#define ELOG(fmt, ...)     f_logger::g_instance().log("ERROR", fmt, ##__VA_ARGS__)

// main code begins here.

// main enum
typedef enum _ALGIS_STATUS : uint32_t {
    ALGIS_OK = 0,
    // common WIN32 errors.
    AL_PROCESS_NOT_FOUND,   //< the target process was not found.
    AL_HANDLE_NOT_FOUND,    //< the target HANDLE was not found.
    AL_INVALID_ADDRESS,     //< the target address was not found.
    AL_INVALID_PARAMETER,   //< ERROR_INVALID_PARAMTER
    AL_OOM,                 //< ERROR_NOT_ENOUGH_MEMORY
    AL_ALREADY_EXISTS,      //< ERROR_FILE_EXISTS
    AL_TIMEOUT,             //< WAIT_TIMEOUT
    AL_FILE_NOT_FOUND,      //< ERROR_FILE_NOT_FOUND

    // hooking/mapping
    AL_ERR_MAPPING_FAILED,
    AL_HOOK_FAILED,

    // IPC section.
    // shared memory
    AL_MAPPING_CREATE_FAILED, //< Failed CreateFileMapping
    AL_MAPPING_OPEN_FAILED,   //< Failed OpenFileMapping
    AL_MAP_VIEW_FAILED,       //< Failed MapViewOfFile
    AL_UNMAP_VIEW_FAILED,     //< Failed UnmapViewOfFile
    AL_WAIT_TIMEOUT,          //< Shared Memory timeout 

    // named pipes error.
    AL_CREATE_FAILED,         //< Failed CreateNamedPipe or CreatePipe (linux counterparts too)
    AL_CONNECT_FAILED,        //< Failed ConnectNamedPipe
    AL_BUSY,                  //< ERROR_PIPE_BUSY
    AL_BROKEN,                //< ERROR_BROKEN_PIPE
    AL_DISCONNECTED,          //< ERROR_PIPE_DISCONNECTED
    AL_MORE_DATA_AVAILABLE,   //< ERROR_MORE_DATA
    AL_MODE_CHANGE_FAILED,    //< Failed SetNamedPipeHandleState
    AL_WRITE_FAILED,          //< failed to send data
    AL_READ_FAILED,           //< failed to read data

    // error for internal errors
    AL_INTERNAL_UNKNOWN_SYSTEM_ERROR
} ALGIS_STATUS;

__attr_pack__ struct n_pipe {
    char magic[10]; // just enough to store "algis_ipc"
    uint32_t payloadSize;
    uint32_t msgType; // optional
}; __attr_pop__

__attr_pack__ struct s_mem {
    char magic[10]; // just enough to store "algis_ipc"
    uint32_t payloadSize;
}; __attr_pop__

// the main namespace, where youll find most of your functions
namespace algis
{
    //> the code in detail is SUPER messy, barely readable and more! if your trying to optimize it or whatever,
    //> might as well just kill yourself cause your not fixing it
    namespace detail {
        /////////////////////////////////////////////////////////////////
        ////////////////// !! START OF IAT HOOKER !! ////////////////////
        /////////////////////////////////////////////////////////////////

#if defined(_OS_WINDOWS)
    // IAT code for windows
#if defined(ALGIS_EXTERNAL)
    // NTAPIs
        namespace ntapi
        {
            // structs or enums neded for the NTAPI functions.
            typedef enum _MEMORY_INFORMATION_CLASS {
                MemoryBasicInformation = 0,
                MemoryWorkingSetInformation = 1,
                MemoryMappedFilenameInformation = 2,
                MemoryRegionInformation = 3,
                MemoryWorkingSetExInformation = 4,
                MemorySharedCommitInformation = 5
            } MEMORY_INFORMATION_CLASS;

            typedef enum _SYSTEM_INFORMATION_CLASS {
                SystemBasicInformation = 0,
                SystemPerformanceInformation = 2,
                SystemTimeOfDayInformation = 3,
                SystemProcessInformation = 5,
                SystemProcessorPerformanceInformation = 8,
                SystemHandleInformation = 16,
                SystemInterruptInformation = 23,
                SystemExceptionInformation = 33,
                SystemRegistryQuotaInformation = 37,
                SystemLookasideInformation = 45,
                SystemCodeIntegrityInformation = 103,
                SystemPolicyInformation = 134
            } SYSTEM_INFORMATION_CLASS, * PSYSTEM_INFORMATION_CLASS;

            typedef enum _SECTION_INHERIT {
                ViewShare = 1,
                ViewUnmap = 2
            } SECTION_INHERIT;

            typedef struct _UNICODE_STRING {
                USHORT Length;
                USHORT MaximumLength;
                PWSTR  Buffer;
            } UNICODE_STRING, * PUNICODE_STRING;

            typedef struct _CLIENT_ID {
                HANDLE UniqueProcess;
                HANDLE UniqueThread;
            } CLIENT_ID, * PCLIENT_ID;

            typedef struct _OBJECT_ATTRIBUTES {
                ULONG Length;
                HANDLE RootDirectory;
                PUNICODE_STRING ObjectName;
                ULONG Attributes;
                PVOID SecurityDescriptor;
                PVOID SecurityQualityOfService;
            } OBJECT_ATTRIBUTES, * POBJECT_ATTRIBUTES;

            typedef struct _SYSTEM_HANDLE {
                ULONG ProcessId;
                BYTE ObjectTypeNumber;
                BYTE Flags;
                USHORT Handle;
                PVOID Object;
                ACCESS_MASK GrantedAccess;
            } SYSTEM_HANDLE, * PSYSTEM_HANDLE;

            typedef struct _SYSTEM_HANDLE_INFORMATION {
                ULONG_PTR HandleCount;
                SYSTEM_HANDLE Handles[1];
            } SYSTEM_HANDLE_INFORMATION, * PSYSTEM_HANDLE_INFORMATION;

            // function defs
            typedef NTSTATUS(NTAPI* pNtReadVirtualMemory)(HANDLE, PVOID, PVOID, SIZE_T, PSIZE_T);
            typedef NTSTATUS(NTAPI* pNtWriteVirtualMemory)(HANDLE, PVOID, PVOID, SIZE_T, PSIZE_T);
            typedef NTSTATUS(NTAPI* pNtProtectVirtualMemory)(HANDLE, PVOID*, PSIZE_T, ULONG, PULONG);
            typedef NTSTATUS(NTAPI* pNtAllocateVirtualMemory)(HANDLE, PVOID*, ULONG_PTR, PSIZE_T, ULONG, ULONG);
            typedef NTSTATUS(NTAPI* pNtClose)(HANDLE);
            typedef NTSTATUS(NTAPI* pNtDuplicateObject)(HANDLE, HANDLE, HANDLE, PHANDLE, ACCESS_MASK, ULONG, ULONG);
            typedef NTSTATUS(NTAPI* pNtQueryVirtualMemory)(HANDLE, PVOID, MEMORY_INFORMATION_CLASS, PVOID, SIZE_T, PSIZE_T);
            typedef NTSTATUS(NTAPI* pNtOpenProcess)(PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES, PCLIENT_ID);
            typedef NTSTATUS(NTAPI* pNtQuerySystemInformation)(SYSTEM_INFORMATION_CLASS, PVOID, ULONG, PULONG);

            // for mapper
            typedef NTSTATUS(NTAPI* pNtCreateSection)(PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES, PLARGE_INTEGER, ULONG, ULONG, HANDLE);
            typedef NTSTATUS(NTAPI* pNtMapViewOfSection)(HANDLE, HANDLE, PVOID*, ULONG_PTR, SIZE_T, PLARGE_INTEGER, PSIZE_T, SECTION_INHERIT, ULONG, ULONG);
            typedef NTSTATUS(NTAPI* pNtUnmapViewOfSection)(HANDLE, PVOID);

            template <typename T>
            inline T FetchNt(const char* functionName) {
                HMODULE hNtdll = ::GetModuleHandleA("ntdll.dll");
                if (!hNtdll) {
                    hNtdll = ::LoadLibraryA("ntdll.dll");
                    if (!hNtdll) return static_cast<T>(nullptr);
                }

                FARPROC proc = ::GetProcAddress(hNtdll, functionName);
                if (!proc) {
                    return static_cast<T>(nullptr);
                }

                return reinterpret_cast<T>(reinterpret_cast<void*>(proc));
            }
        } // namespace ntapi
        class Process {
        private:
            std::string name;
            DWORD pid;
            HANDLE handle;

            ntapi::pNtReadVirtualMemory     _NtReadVirtualMemory;
            ntapi::pNtWriteVirtualMemory    _NtWriteVirtualMemory;
            ntapi::pNtProtectVirtualMemory  _NtProtectVirtualMemory;
            ntapi::pNtAllocateVirtualMemory _NtAllocateVirtualMemory;
            ntapi::pNtOpenProcess           _NtOpenProcess;
            ntapi::pNtClose                 _NtClose;
            ntapi::pNtDuplicateObject       _NtDuplicateObject;

            HANDLE hijack_handle(DWORD targetPid);
            DWORD GetPID(std::string targetName);

        public:
            Process(std::string targetName);
            ~Process();

            Process(const Process& other) = delete;
            Process& operator=(const Process& other) = delete;

            inline Process(Process&& other) noexcept
                : name(std::move(other.name)), pid(other.pid), handle(other.handle),
                _NtReadVirtualMemory(other._NtReadVirtualMemory),
                _NtWriteVirtualMemory(other._NtWriteVirtualMemory),
                _NtProtectVirtualMemory(other._NtProtectVirtualMemory),
                _NtAllocateVirtualMemory(other._NtAllocateVirtualMemory),
                _NtOpenProcess(other._NtOpenProcess),
                _NtClose(other._NtClose),
                _NtDuplicateObject(other._NtDuplicateObject) {
                other.pid = 0;
                other.handle = nullptr;
            }

            inline Process& operator=(Process&& other) noexcept {
                if (this != &other) {
                    if (handle && _NtClose) _NtClose(handle);
                    name = std::move(other.name);
                    pid = other.pid;
                    handle = other.handle;
                    _NtReadVirtualMemory = other._NtReadVirtualMemory;
                    _NtWriteVirtualMemory = other._NtWriteVirtualMemory;
                    _NtProtectVirtualMemory = other._NtProtectVirtualMemory;
                    _NtAllocateVirtualMemory = other._NtAllocateVirtualMemory;
                    _NtOpenProcess = other._NtOpenProcess;
                    _NtClose = other._NtClose;
                    _NtDuplicateObject = other._NtDuplicateObject;
                    other.pid = 0;
                    other.handle = nullptr;
                }
                return *this;
            }

            HANDLE get_handle();
            DWORD get_pid();

            NTSTATUS read_mem(PVOID base, PVOID buf, SIZE_T size, PSIZE_T read);
            NTSTATUS write_mem(PVOID base, PVOID buf, SIZE_T size, PSIZE_T written);
            NTSTATUS change_protection(PVOID base, SIZE_T size, ULONG prot, PULONG oldProt);
            PVOID allocate_mem(SIZE_T size, ULONG type, ULONG prot);

            void SuspendAllThreads(DWORD processId, bool suspend);

            struct Module {
                std::string name;
                BYTE* base;
                int size;
            };
        };

        // function defs of Process class outside (must be marked inline for header use only)
        inline HANDLE Process::get_handle() { return this->handle; }
        inline DWORD Process::get_pid() { return this->pid; }

        inline DWORD Process::GetPID(std::string name) {
            HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
            if (snapshot == INVALID_HANDLE_VALUE) return 0;

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
            HMODULE ntdll = GetModuleHandleA("ntdll.dll");
            typedef NTSTATUS(NTAPI* _RtlAdjustPrivilege)(ULONG, BOOLEAN, BOOLEAN, PBOOLEAN);
            auto RtlAdjustPrivilege = (_RtlAdjustPrivilege)GetProcAddress(ntdll, "RtlAdjustPrivilege");
            auto myNtQuery = (ntapi::pNtQuerySystemInformation)GetProcAddress(ntdll, "NtQuerySystemInformation");

            BOOLEAN oldPriv;
            if (RtlAdjustPrivilege) {
                RtlAdjustPrivilege(20, TRUE, FALSE, &oldPriv);
            }

            NTSTATUS status = 0;
            ULONG size = 0x10000;
            ntapi::PSYSTEM_HANDLE_INFORMATION hInfo = (ntapi::PSYSTEM_HANDLE_INFORMATION)VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

            while (myNtQuery && (status = myNtQuery(ntapi::SystemHandleInformation, hInfo, size, &size)) == 0xC0000004) {
                VirtualFree(hInfo, 0, MEM_RELEASE);
                size *= 2;
                hInfo = (ntapi::PSYSTEM_HANDLE_INFORMATION)VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
            }

            if (!NT_SUCCESS(status) || !hInfo) {
                if (hInfo) VirtualFree(hInfo, 0, MEM_RELEASE);
                return nullptr;
            }

            HANDLE result = nullptr;
            ntapi::OBJECT_ATTRIBUTES objAttr = { sizeof(ntapi::OBJECT_ATTRIBUTES) };

            for (ULONG i = 0; i < hInfo->HandleCount; i++) {
                ntapi::SYSTEM_HANDLE sh = hInfo->Handles[i];
                if (!sh.Handle) continue;

                ntapi::CLIENT_ID CID = { (HANDLE)(ULONG_PTR)sh.ProcessId, nullptr };
                HANDLE ownerHandle = nullptr;

                status = _NtOpenProcess(&ownerHandle, PROCESS_DUP_HANDLE, &objAttr, &CID);
                if (!NT_SUCCESS(status) || !ownerHandle) continue;

                HANDLE dupHandle = nullptr;
                status = _NtDuplicateObject(ownerHandle, (HANDLE)(ULONG_PTR)sh.Handle, GetCurrentProcess(), &dupHandle, PROCESS_ALL_ACCESS, 0, 0);
                _NtClose(ownerHandle);

                if (NT_SUCCESS(status) && dupHandle && GetProcessId(dupHandle) == targetPid) {
                    result = dupHandle;
                    break;
                }
                if (dupHandle) _NtClose(dupHandle);
            }

            VirtualFree(hInfo, 0, MEM_RELEASE);
            return result;
        }
        inline Process::Process(std::string targetName) {
            _NtReadVirtualMemory = ntapi::FetchNt<ntapi::pNtReadVirtualMemory>("NtReadVirtualMemory");
            _NtWriteVirtualMemory = ntapi::FetchNt<ntapi::pNtWriteVirtualMemory>("NtWriteVirtualMemory");
            _NtProtectVirtualMemory = ntapi::FetchNt<ntapi::pNtProtectVirtualMemory>("NtProtectVirtualMemory");
            _NtAllocateVirtualMemory = ntapi::FetchNt<ntapi::pNtAllocateVirtualMemory>("NtAllocateVirtualMemory");
            _NtOpenProcess = ntapi::FetchNt<ntapi::pNtOpenProcess>("NtOpenProcess");
            _NtClose = ntapi::FetchNt<ntapi::pNtClose>("NtClose");
            _NtDuplicateObject = ntapi::FetchNt<ntapi::pNtDuplicateObject>("NtDuplicateObject");

            this->name = targetName;
            this->pid = GetPID(targetName);
            this->handle = hijack_handle(this->pid);

            if (!this->handle) {
                std::cerr << "failed to hijack handle for: " << targetName << std::endl;
            }
        }
        inline Process::~Process() {
            if (handle && _NtClose) _NtClose(handle);
        }

        inline NTSTATUS Process::read_mem(PVOID base, PVOID buf, SIZE_T size, PSIZE_T read) {
            return _NtReadVirtualMemory ? _NtReadVirtualMemory(this->handle, base, buf, size, read) : STATUS_INVALID_HANDLE;
        }
        inline NTSTATUS Process::write_mem(PVOID base, PVOID buf, SIZE_T size, PSIZE_T written) {
            return _NtWriteVirtualMemory ? _NtWriteVirtualMemory(this->handle, base, buf, size, written) : STATUS_INVALID_HANDLE;
        }
        inline NTSTATUS Process::change_protection(PVOID base, SIZE_T size, ULONG prot, PULONG oldProt) {
            PVOID addressToProtect = base;
            SIZE_T regionSize = size;
            return _NtProtectVirtualMemory ? _NtProtectVirtualMemory(this->handle, &addressToProtect, &regionSize, prot, oldProt) : STATUS_INVALID_HANDLE;
        }
        inline PVOID Process::allocate_mem(SIZE_T size, ULONG type, ULONG prot) {
            PVOID base = nullptr;
            SIZE_T regionSize = size;
            if (_NtAllocateVirtualMemory) {
                _NtAllocateVirtualMemory(this->handle, &base, 0, &regionSize, type, prot);
            }
            return base;
        }

        inline void Process::SuspendAllThreads(DWORD processId, bool suspend) {
            HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
            if (hSnapshot == INVALID_HANDLE_VALUE) return;

            THREADENTRY32 te;
            te.dwSize = sizeof(THREADENTRY32);

            if (Thread32First(hSnapshot, &te)) {
                do {
                    if (te.th32OwnerProcessID == processId) {
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

        // MMapper
        class mapper
        {
        private:
            Process& process;
            bool fix_relocations(PVOID localBase, uintptr_t remoteBase);
            bool resolve_imports(PVOID localBase);
            bool register_seh(PVOID localBase, uintptr_t remoteBase, SIZE_T imageSize);

        public:
            mapper(Process& process);

            struct MapResult {
                uintptr_t remoteBase;
                uintptr_t entryPoint;
                bool success;
            };

            MapResult map(const std::wstring& dllPath);
        };

        // manual mapper defs
        mapper::mapper(Process& process) : process(process) {}
        // fix relocations
        bool mapper::fix_relocations(PVOID localBase, uintptr_t remoteBase) {
            auto dosHeader = (PIMAGE_DOS_HEADER)localBase;
            auto ntHeaders = (PIMAGE_NT_HEADERS)((uintptr_t)localBase + dosHeader->e_lfanew);

            uintptr_t delta = remoteBase - ntHeaders->OptionalHeader.ImageBase;
            if (delta == 0) return true;

            auto relocDir = &ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];
            if (relocDir->VirtualAddress == 0) return true;

            auto reloc = (PIMAGE_BASE_RELOCATION)((uintptr_t)localBase + relocDir->VirtualAddress);

            while (reloc->VirtualAddress) {
                DWORD entryCount = (reloc->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);
                auto entries = (PWORD)(reloc + 1);

                for (DWORD i = 0; i < entryCount; i++) {
                    if ((entries[i] >> 12) == IMAGE_REL_BASED_DIR64) {
                        uintptr_t* patch = (uintptr_t*)((uintptr_t)localBase + reloc->VirtualAddress + (entries[i] & 0xFFF));
                        *patch += delta;
                    }
                }
                reloc = (PIMAGE_BASE_RELOCATION)((uintptr_t)reloc + reloc->SizeOfBlock);
            }
            return true;
        }
        // resolve imports
        bool mapper::resolve_imports(PVOID localBase) {
            auto dosHeader = (PIMAGE_DOS_HEADER)localBase;
            auto ntHeaders = (PIMAGE_NT_HEADERS)((uintptr_t)localBase + dosHeader->e_lfanew);

            auto importDir = &ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
            if (importDir->VirtualAddress == 0) return true;

            auto importDesc = (PIMAGE_IMPORT_DESCRIPTOR)((uintptr_t)localBase + importDir->VirtualAddress);

            while (importDesc->Name) {
                const char* moduleName = (const char*)((uintptr_t)localBase + importDesc->Name);
                HMODULE mod = LoadLibraryA(moduleName);
                if (!mod) return false;

                auto thunk = (PIMAGE_THUNK_DATA)((uintptr_t)localBase + importDesc->FirstThunk);
                auto origThunk = (PIMAGE_THUNK_DATA)((uintptr_t)localBase + importDesc->OriginalFirstThunk);

                while (thunk->u1.AddressOfData) {
                    if (IMAGE_SNAP_BY_ORDINAL(origThunk->u1.Ordinal)) {
                        thunk->u1.Function = (uintptr_t)GetProcAddress(mod, (LPCSTR)IMAGE_ORDINAL(origThunk->u1.Ordinal));
                    }
                    else {
                        auto importByName = (PIMAGE_IMPORT_BY_NAME)((uintptr_t)localBase + origThunk->u1.AddressOfData);
                        thunk->u1.Function = (uintptr_t)GetProcAddress(mod, importByName->Name);
                    }
                    if (!thunk->u1.Function) return false;
                    thunk++;
                    origThunk++;
                }
                importDesc++;
            }
            return true;
        }
        // register seh
        bool mapper::register_seh(PVOID localBase, uintptr_t remoteBase, SIZE_T imageSize) {
            auto dosHeader = (PIMAGE_DOS_HEADER)localBase;
            auto ntHeaders = (PIMAGE_NT_HEADERS)((uintptr_t)localBase + dosHeader->e_lfanew);

            auto exceptionDir = &ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION];
            if (exceptionDir->VirtualAddress == 0) return true;

            auto funcTable = (PRUNTIME_FUNCTION)((uintptr_t)localBase + exceptionDir->VirtualAddress);
            DWORD entryCount = exceptionDir->Size / sizeof(RUNTIME_FUNCTION);

            return RtlAddFunctionTable(funcTable, entryCount, remoteBase);
        }
        // main mapper function
        mapper::MapResult mapper::map(const std::wstring& dllPath) {
            MapResult result = { 0, 0, false };

            HMODULE ntdll = GetModuleHandleA("ntdll.dll");
            auto _NtCreateSection = (ntapi::pNtCreateSection)GetProcAddress(ntdll, "NtCreateSection");
            auto _NtMapViewOfSection = (ntapi::pNtMapViewOfSection)GetProcAddress(ntdll, "NtMapViewOfSection");
            auto _NtUnmapViewOfSection = (ntapi::pNtUnmapViewOfSection)GetProcAddress(ntdll, "NtUnmapViewOfSection");

            // Safety check
            if (!_NtCreateSection || !_NtMapViewOfSection || !_NtUnmapViewOfSection) return result;

            // read dll from disk
            HANDLE file = CreateFileW(dllPath.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
            if (file == INVALID_HANDLE_VALUE) return result;

            DWORD fileSize = GetFileSize(file, nullptr);
            std::vector<BYTE> dllBuffer(fileSize);
            DWORD bytesRead;
            ReadFile(file, dllBuffer.data(), fileSize, &bytesRead, nullptr);
            CloseHandle(file);

            auto dosHeader = (PIMAGE_DOS_HEADER)dllBuffer.data();
            auto ntHeaders = (PIMAGE_NT_HEADERS)(dllBuffer.data() + dosHeader->e_lfanew);
            SIZE_T imageSize = ntHeaders->OptionalHeader.SizeOfImage;

            // create section
            HANDLE section = nullptr;
            LARGE_INTEGER sectionSize;
            sectionSize.QuadPart = (LONGLONG)imageSize;
            NTSTATUS status = _NtCreateSection(&section, SECTION_ALL_ACCESS, nullptr, &sectionSize, PAGE_EXECUTE_READWRITE, SEC_COMMIT, nullptr);
            if (!NT_SUCCESS(status)) return result;

            // map locally
            PVOID localBase = nullptr;
            SIZE_T viewSize = imageSize;
            status = _NtMapViewOfSection(section, GetCurrentProcess(), &localBase, 0, imageSize, nullptr, &viewSize, ntapi::ViewUnmap, 0, PAGE_EXECUTE_READWRITE);
            if (!NT_SUCCESS(status)) { CloseHandle(section); return result; }

            // map remotely
            PVOID remoteBase = nullptr;
            viewSize = imageSize;
            status = _NtMapViewOfSection(section, process.get_handle(), &remoteBase, 0, imageSize, nullptr, &viewSize, ntapi::ViewUnmap, 0, PAGE_EXECUTE_READWRITE);
            if (!NT_SUCCESS(status)) { _NtUnmapViewOfSection(GetCurrentProcess(), localBase); CloseHandle(section); return result; }

            // copy headers
            memcpy(localBase, dllBuffer.data(), ntHeaders->OptionalHeader.SizeOfHeaders);

            // copy sections
            auto section_header = IMAGE_FIRST_SECTION(ntHeaders);
            for (WORD i = 0; i < ntHeaders->FileHeader.NumberOfSections; i++) {
                if (section_header[i].SizeOfRawData == 0) { section_header++; continue; }
                PVOID dest = (PVOID)((uintptr_t)localBase + section_header[i].VirtualAddress);
                PVOID src = (PVOID)(dllBuffer.data() + section_header[i].PointerToRawData);
                memcpy(dest, src, section_header[i].SizeOfRawData);
            }

            // fix relocations and imports against remote base
            fix_relocations(localBase, (uintptr_t)remoteBase);
            resolve_imports(localBase);
            register_seh(localBase, (uintptr_t)remoteBase, imageSize);

            // get exported function RVA
            auto exportDir = &ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
            if (exportDir->VirtualAddress != 0) {
                auto exportTable = (PIMAGE_EXPORT_DIRECTORY)((uintptr_t)localBase + exportDir->VirtualAddress);
                auto names = (DWORD*)((uintptr_t)localBase + exportTable->AddressOfNames);
                auto functions = (DWORD*)((uintptr_t)localBase + exportTable->AddressOfFunctions);
                auto ordinals = (WORD*)((uintptr_t)localBase + exportTable->AddressOfNameOrdinals);

                for (DWORD i = 0; i < exportTable->NumberOfNames; i++) {
                    const char* name = (const char*)((uintptr_t)localBase + names[i]);
                    // !! CHANGE THIS IF YOU WANT !!
                    if (strcmp(name, "ThreadpoolCallback") == 0) {
                        result.entryPoint = (uintptr_t)remoteBase + functions[ordinals[i]];
                        break;
                    }
                }
            }


            result.remoteBase = (uintptr_t)remoteBase;
            result.success = true;

            // unmap local view, remote stays mapped
            _NtUnmapViewOfSection(GetCurrentProcess(), localBase);

            return result;
        }

        // IAT hooker
        struct Hook {
            void** targetSlot; /* Address of the pointer we're replacing. */
            void* orig; /* Original function pointer. */

            /* Hook. */
            bool hook(Process& proc, void* jmpInstruction, void* newTarget)
            {
                /*
                * Calculate the address of the pointer slot.
                * jmp [rip + offset] - offset is at byte 3. Pointer is at instruction + 7 + offset.
                */
                int32_t offset = 0;
                SIZE_T read;
                proc.read_mem((PVOID)((uintptr_t)jmpInstruction + 3), &offset, sizeof(int32_t), &read);
                targetSlot = (void**)((uintptr_t)jmpInstruction + 7 + offset);

                /* Read original memory. */
                proc.read_mem(targetSlot, &orig, sizeof(void*), &read);

                /* Write the new target. */
                ULONG oldProt;
                proc.change_protection(targetSlot, sizeof(void*), PAGE_EXECUTE_READWRITE, &oldProt);
                SIZE_T written;
                proc.write_mem(targetSlot, &newTarget, sizeof(void*), &written);
                proc.change_protection(targetSlot, sizeof(void*), oldProt, &oldProt);

                /* We are hooked! */
                return true;
            }

            /* Unhook. */
            bool unhook(Process& proc)
            {
                ULONG oldProt;
                proc.change_protection(targetSlot, sizeof(void*), PAGE_EXECUTE_READWRITE, &oldProt);
                SIZE_T written;
                proc.write_mem(targetSlot, &orig, sizeof(void*), &written);
                proc.change_protection(targetSlot, sizeof(void*), oldProt, &oldProt);
                /* We are unhooked! */
                return true;
            }
        };
        struct HookLib {
            Process* process = nullptr;
            std::map<void*, Hook*> registry;
            mapper::MapResult lastMapResult = { 0,0,false };
        };
        inline HookLib& GetHookLib() {
            static HookLib g_Lib;
            return g_Lib;
        }

        // finished IAT hooker (hopefully)
        inline ALGIS_STATUS __stdcall _al_hk_init(const char* processName) {
            auto& lib = GetHookLib();
            if (lib.process) return AL_INTERNAL_UNKNOWN_SYSTEM_ERROR;

            lib.process = new Process(std::string(processName));

            if (lib.process->get_pid() == 0) {
                delete lib.process;
                lib.process = nullptr;
                return AL_PROCESS_NOT_FOUND;
            }
            return ALGIS_OK;
        }

        inline ALGIS_STATUS __stdcall _al_hk_uninit() {
            auto& lib = GetHookLib();
            delete lib.process;
            lib.process = nullptr;
            return ALGIS_OK;
        }

        inline ALGIS_STATUS __stdcall _al_map(const wchar_t* dllPath, uintptr_t* oEntryPoint) {
            auto& lib = GetHookLib();
            if (!lib.process) return AL_INTERNAL_UNKNOWN_SYSTEM_ERROR;

            mapper mmap(*lib.process);
            lib.lastMapResult = mmap.map(dllPath);

            if (!lib.lastMapResult.success) {
                return AL_ERR_MAPPING_FAILED;
            }

            if (oEntryPoint) {
                *oEntryPoint = lib.lastMapResult.entryPoint;
            }
            return ALGIS_OK;
        }

        inline ALGIS_STATUS __stdcall _a_create_hook(void* jmpInstruction, void* newTarget, A_HOOK* oHook) {
            auto& lib = GetHookLib();
            if (!lib.process) return AL_INTERNAL_UNKNOWN_SYSTEM_ERROR;
            if (lib.registry.find(jmpInstruction) != lib.registry.end()) return AL_HOOK_FAILED;

            DWORD targetPID = lib.process->get_pid();
            lib.process->SuspendAllThreads(targetPID, true);

            Hook* h = new Hook();
            bool success = h->hook(*lib.process, jmpInstruction, newTarget);

            lib.process->SuspendAllThreads(targetPID, false);

            if (!success) {
                delete h;
                return AL_HOOK_FAILED;
            }

            lib.registry[jmpInstruction] = h;
            *oHook = (void*)h;

            return ALGIS_OK;
        }

        inline ALGIS_STATUS __stdcall _a_destroy_hook(void* hook) {
            auto& lib = GetHookLib();
            if (!lib.process) return AL_INTERNAL_UNKNOWN_SYSTEM_ERROR;

            DWORD targetPID = lib.process->get_pid();
            lib.process->SuspendAllThreads(targetPID, true);

            if (hook != H_DESTROY) {
                Hook* h = (Hook*)hook;
                h->unhook(*lib.process);

                for (auto it = lib.registry.begin(); it != lib.registry.end(); ++it) {
                    if (it->second == h) {
                        lib.registry.erase(it);
                        break;
                    }
                }
                delete h;
            }
            else {
                for (auto& pair : lib.registry) {
                    Hook* hookObj = pair.second;
                    hookObj->unhook(*lib.process);
                    delete hookObj;
                }
                lib.registry.clear();
            }

            lib.process->SuspendAllThreads(targetPID, false);
            return ALGIS_OK;
        }
#endif // IAT hooker
#endif // IAT hooker

        /////////////////////////////////////////////////////////////////
        ////////////////// !! START OF INTERNAL HELPERS !! //////////////
        /////////////////////////////////////////////////////////////////

#if defined(_OS_WINDOWS) //< @begin _OS_WINDOWS
    #if defined(ALGIS_INTERNAL) //< @begin ALGIS_INTERNAL
    // create threadpool 
    // if mmaped, you should use _CRT_INIT for windows
    // read/write functions

        // unnamed namespace for internal anoynomus functions
        namespace {
            static bool safe_read(uintptr_t address, void* buffer, size_t size)
            {
                if (!address || address < 0x10000 || address > 0x7FFFFFFFFFFF) return false;
                if (IsBadReadPtr(reinterpret_cast<void*>(address), size)) return false;
                __try
                {
                    CopyMemory(buffer, reinterpret_cast<void*>(address), size);
                    return true;
                }
                __except (EXCEPTION_EXECUTE_FAULT)
                {
                    return false;
                }
            }

            static bool safe_write(uintptr_t address, const void* buffer, size_t size)
            {
                if (!address || address < 0x10000 || address > 0x7FFFFFFFFFFF) return false;
                if (IsBadWritePtr(reinterpret_cast<void*>(address), size)) return false;
                __try
                {
                    CopyMemory(reinterpret_cast<void*>(address), buffer, size);
                    return true;
                }
                __except (EXCEPTION_EXECUTE_HANDLER)
                {
                    return false;
                }
            }

            struct HookEntry
            {
                void* detour;
                void*** classInstance;
                void** originalVmt;
                void** customVmt;
                void* originalFunc;
                size_t index;
                size_t totalCount;
                bool isEnabled;
            };

            inline std::vector<HookEntry> g_Hooks;

            inline bool IsDataCaveOverlap(uintptr_t start, size_t size) {
                uintptr_t end = start + size;
                for (const auto& entry : g_Hooks) {
                    if (entry.customVmt) {
                        uintptr_t usedStart = reinterpret_cast<uintptr_t>(&(entry.customVmt[-2]));
                        uintptr_t usedEnd = usedStart + ((entry.totalCount + 2) * sizeof(void*));

                        if (start < usedEnd && end > usedStart) {
                            return true;
                        }
                    }
                }
                return false;
            }

            inline void GetTextSectionRange(uintptr_t moduleBase, uintptr_t& oStart, uintptr_t& oEnd) {
                auto dosHeader = (PIMAGE_DOS_HEADER)moduleBase;
                if (!dosHeader || dosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
                    return;
                }

                auto ntHeaders = (PIMAGE_NT_HEADERS)(moduleBase + dosHeader->e_lfanew);
                if (!ntHeaders || ntHeaders->Signature != IMAGE_NT_SIGNATURE) {
                    return;
                }

                oStart = moduleBase + ntHeaders->OptionalHeader.BaseOfCode;
                oEnd = oStart + ntHeaders->OptionalHeader.SizeOfCode;

                auto sectionHeader = IMAGE_FIRST_SECTION(ntHeaders);
                for (WORD i = 0; i < ntHeaders->FileHeader.NumberOfSections; i++) {
                    if (sectionHeader[i].Characteristics & IMAGE_SCN_MEM_EXECUTE) {
                        uintptr_t secStart = moduleBase + sectionHeader[i].VirtualAddress;
                        uintptr_t secEnd = secStart + sectionHeader[i].Misc.VirtualSize;

                        if (secStart < oStart) oStart = secStart;
                        if (secEnd > oEnd) oEnd = secEnd;
                    }
                }
            }

            inline size_t GetVMTSizeDynamic(void** vmt) {
                static uintptr_t textStart = 0, textEnd = 0;
                if (!textStart) {
                    GetTextSectionRange((uintptr_t)GetModuleHandleA(NULL), textStart, textEnd);
                }

                if (!textStart || !vmt) {
                    return 0;
                }

                size_t count = 0;
                while (vmt[count]) {
                    uintptr_t func = (uintptr_t)vmt[count];
                    if (func < textStart || func >= textEnd) {
                        break;
                    }
                    count++;
                }
                return count;
            }

            inline uintptr_t FindGameDataCave(uintptr_t moduleBase, size_t size) {
                auto dosHeader = (PIMAGE_DOS_HEADER)moduleBase;
                if (!dosHeader || dosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
                    return 0;
                }

                auto ntHeaders = (PIMAGE_NT_HEADERS)(moduleBase + dosHeader->e_lfanew);
                if (!ntHeaders || ntHeaders->Signature != IMAGE_NT_SIGNATURE) {
                    return 0;
                }

                auto sectionHeader = IMAGE_FIRST_SECTION(ntHeaders);

                for (WORD i = 0; i < ntHeaders->FileHeader.NumberOfSections; i++) {
                    if (memcmp(sectionHeader[i].Name, ".data", 5) == 0 ||
                        memcmp(sectionHeader[i].Name, ".bss", 4) == 0) {

                        uintptr_t sectionStart = moduleBase + sectionHeader[i].VirtualAddress;
                        size_t sectionSize = sectionHeader[i].Misc.VirtualSize;

                        size_t consecutiveZeroes = 0;
                        for (size_t offset = sectionSize - 1; offset > 0; offset--) {
                            if (*(BYTE*)(sectionStart + offset) == 0x00) {
                                consecutiveZeroes++;
                                if (consecutiveZeroes >= size + 16) {
                                    uintptr_t candidate = (sectionStart + offset + 15) & ~15;

                                    if (IsDataCaveOverlap(candidate, size)) {
                                        consecutiveZeroes = 0;
                                        continue;
                                    }

                                    return candidate;
                                }
                            }
                            else {
                                consecutiveZeroes = 0;
                            }
                        }
                    }
                }
                return 0;
            }

            inline HookEntry* FindHookEntry(void* detour) {
                for (auto& entry : g_Hooks) {
                    if (entry.detour == detour) {
                        return &entry;
                    }
                }
                return nullptr;
            }

            inline bool CreateHook(void* detour, void* classInstance, size_t index, void** outOriginal = nullptr) {
                if (!detour || !classInstance) {
                    return false;
                }

                if (FindHookEntry(detour) != nullptr) {
                    return false;
                }

                void*** pInstance = reinterpret_cast<void***>(classInstance);
                void** originalVmt = *pInstance;

                if (!originalVmt) {
                    return false;
                }

                size_t totalCount = GetVMTSizeDynamic(originalVmt);
                if (index >= totalCount || totalCount == 0) {
                    return false;
                }

                uintptr_t gameModule = (uintptr_t)GetModuleHandleA(NULL);
                size_t totalVmtBytes = (totalCount + 2) * sizeof(void*);

                uintptr_t dataCaveRaw = FindGameDataCave(gameModule, totalVmtBytes);
                if (!dataCaveRaw) {
                    return false;
                }

                // Ensure the Data Cave memory page is writable
                DWORD oldProtect = 0;
                if (!VirtualProtect((LPVOID)dataCaveRaw, totalVmtBytes, PAGE_READWRITE, &oldProtect)) {
                    return false;
                }

                void** shadowVMTBuffer = reinterpret_cast<void**>(dataCaveRaw);

                shadowVMTBuffer[0] = originalVmt[-2];
                shadowVMTBuffer[1] = originalVmt[-1];

                void** customVmt = &shadowVMTBuffer[2];
                std::memcpy(customVmt, originalVmt, totalCount * sizeof(void*));
                customVmt[index] = detour;

                // Restore original protection on Data Cave page
                VirtualProtect((LPVOID)dataCaveRaw, totalVmtBytes, oldProtect, &oldProtect);

                if (outOriginal) {
                    *outOriginal = originalVmt[index];
                }

                HookEntry entry;
                entry.detour = detour;
                entry.classInstance = pInstance;
                entry.originalVmt = originalVmt;
                entry.customVmt = customVmt;
                entry.originalFunc = originalVmt[index];
                entry.index = index;
                entry.totalCount = totalCount;
                entry.isEnabled = false;

                g_Hooks.push_back(entry);
                return true;
            }

            inline bool EnableHook(void* detour) {
                HookEntry* entry = FindHookEntry(detour);
                if (!entry || entry->isEnabled) {
                    return false;
                }

                DWORD oldProtect = 0;
                if (!VirtualProtect(entry->classInstance, sizeof(void*), PAGE_READWRITE, &oldProtect)) {
                    return false;
                }

                *(entry->classInstance) = entry->customVmt;
                VirtualProtect(entry->classInstance, sizeof(void*), oldProtect, &oldProtect);

                entry->isEnabled = true;
                return true;
            }

            inline bool DisableHook(void* detour) {
                HookEntry* entry = FindHookEntry(detour);
                if (!entry || !entry->isEnabled) {
                    return false;
                }

                DWORD oldProtect = 0;
                if (!VirtualProtect(entry->classInstance, sizeof(void*), PAGE_READWRITE, &oldProtect)) {
                    return false;
                }

                *(entry->classInstance) = entry->originalVmt;
                VirtualProtect(entry->classInstance, sizeof(void*), oldProtect, &oldProtect);

                entry->isEnabled = false;
                return true;
            }

            inline bool RemoveHook(void* detour) {
                for (auto it = g_Hooks.begin(); it != g_Hooks.end(); ++it) {
                    if (it->detour == detour) {
                        if (it->isEnabled) {
                            DWORD oldProtect = 0;
                            if (VirtualProtect(it->classInstance, sizeof(void*), PAGE_READWRITE, &oldProtect)) {
                                *(it->classInstance) = it->originalVmt;
                                VirtualProtect(it->classInstance, sizeof(void*), oldProtect, &oldProtect);
                            }
                        }

                        void** shadowBuffer = &(it->customVmt[-2]);
                        size_t totalBytes = (it->totalCount + 2) * sizeof(void*);

                        DWORD oldProtect = 0;
                        if (VirtualProtect(shadowBuffer, totalBytes, PAGE_READWRITE, &oldProtect)) {
                            std::memset(shadowBuffer, 0, totalBytes);
                            VirtualProtect(shadowBuffer, totalBytes, oldProtect, &oldProtect);
                        }

                        g_Hooks.erase(it);
                        return true;
                    }
                }
                return false;
            }

            inline void* GetOriginal(void* detour) {
                HookEntry* entry = FindHookEntry(detour);
                return entry ? entry->originalFunc : nullptr;
            }
        }

        // _create_tp_
        inline ALGIS_STATUS _create_tp_(PTP_WORK_CALLBACK callback, PVOID context = NULL) {
            PTP_WORK work = CreateThreadPoolWork(callback, context, NULL);
            if (work) {
                SubmitThreadpoolWork(work);
            }
        }

        inline uintptr_t _get_module_base_() {
            return (uintptr_t)GetModuleHandleA(NULL);
        }

        // READ/WRITE functions
        template<typename T>
        inline T _read_(uintptr_t address) {
            T value{};
            safe_read(address, &value, sizeof(T));
            return value;
        }

        template<typename T>
        inline bool _write_(uintptr_t address, T value) {
            return safe_write(address, &value, sizeof(T));
        }

        inline BOOL _createVHook_(void* detour, void* classInstance, size_t index, void** outOriginal = nullptr) {
            return CreateHook(detour, classInstance, index, outOriginal) ? TRUE : FALSE;
        }

        inline BOOL _enableVHook_(void* detour) {
            return EnableHook(detour) ? TRUE : FALSE;
        }

        inline BOOL _disableVHook_(void* detour) {
            return DisableHook(detour) ? TRUE : FALSE;
        }

        inline BOOL _removeVHook_(void* detour) {
            return RemoveHook(detour) ? TRUE : FALSE;
        }

        inline void* _getVOrignal(void* detour) {
            return GetOriginal(detour);
        }

  
    #elif defined(ALGIS_EXTERNAL) //< @elif ALGIS_EXTERNAL



    #endif //< @end ALGIS_INTERNAL and ALGIS_EXTERNAL
#endif //< @end _OS_WINDOWS

    /////////////////////////////////////////////////////////////////
    ////////////////// !! START OF IPC HELPER !! ////////////////////
    /////////////////////////////////////////////////////////////////

#if defined(_OS_WINDOWS) //< @begin _OS_WINDOWS
    namespace {
        bool IsValidPipeName(LPCWSTR name) {
            return name != nullptr && name[0] != L'\0';
        }
    }
    ALGIS_STATUS __stdcall _createN_pipe_(LPCWSTR pipeName, DWORD bufferSize, HANDLE* oPipeHandle) {
        if (!IsValidPipeName(pipeName) || !oPipeHandle) {
            return AL_INVALID_PARAMETER;
        }

        HANDLE hPipe = CreateNamedPipeW(
            pipeName,
            PIPE_ACCESS_DUPLEX,
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
            PIPE_UNLIMITED_INSTANCES,
            bufferSize, bufferSize,
            0,
            NULL
        );

        if (hPipe == INVALID_HANDLE_VALUE) {
            std::wcerr << L"CreateNamedPipe failed, error" << GetLastError() << '\n';
            return AL_CREATE_FAILED;
        }

        *oPipeHandle = hPipe;
        return ALGIS_OK;
   }

    ALGIS_STATUS __stdcall _openN_pipe_(LPCWSTR pipeName, DWORD timeoutMs, HANDLE* oPipeHandle) {
        if (!IsValidPipeName(pipeName) || !oPipeHandle) {
            return AL_INVALID_PARAMETER;
        }

        *oPipeHandle = INVALID_HANDLE_VALUE;

        if (!WaitNamedPipeW(pipeName, timeoutMs)) {
            std::wcerr << L"WaitNamedPipeW timed out/failed, error: " << GetLastError() << '\n';
            return AL_TIMEOUT;
        }

        HANDLE hPipe = CreateFileW(
            pipeName,
            GENERIC_READ | GENERIC_WRITE,
            0,
            NULL,
            OPEN_EXISTING,
            0,
            NULL
        );

        if (hPipe == INVALID_HANDLE_VALUE) {
            std::wcerr << L"CreateFileW failed for pipe, Error: " << GetLastError() << '\n';
            return AL_CONNECT_FAILED;
        }

        DWORD mode = PIPE_READMODE_MESSAGE;
        if (!SetNamedPipeHandleState(hPipe, &mode, NULL, NULL)) {
            std::wcerr << L"SetNamedPipeHandleState failed, Error: " << GetLastError() << '\n';
            CloseHandle(hPipe);
            return AL_MODE_CHANGE_FAILED;
        }

        *oPipeHandle = hPipe;
        return ALGIS_OK;
    }

    ALGIS_STATUS __stdcall _destroyN_pipe_(HANDLE* pipeHandle) {
        if (!pipeHandle || *pipeHandle == INVALID_HANDLE_VALUE || *pipeHandle == NULL) {
            return AL_INVALID_PARAMETER;
        }

        HANDLE hPipe = *pipeHandle;
        *pipeHandle = INVALID_HANDLE_VALUE;

        DisconnectNamedPipe(hPipe);

        if (!CloseHandle(hPipe)) {
            return AL_BROKEN;
        }

        return ALGIS_OK;
    }



    // XSend and XRecieve
    ALGIS_STATUS __stdcall _xsend_(int transportType, LPCWSTR ipcName, const void* data, size_t dataSize) {
        if (!ipcName || !data || dataSize == 0) {
            return AL_INVALID_PARAMETER;
        }

        // Named pipes
        if (transportType == XPIPE) {
            HANDLE hPipe = INVALID_HANDLE_VALUE;

            ALGIS_STATUS status = _openN_pipe_(ipcName, 5000, &hPipe);
            if (status != ALGIS_OK) {
                return status;
            }

            n_pipe header = {};
            memcpy(header.magic, IPC_MAGIC, sizeof(header.magic));
            header.payloadSize = static_cast<uint32_t>(dataSize);

            std::vector<BYTE> packetBuffer(sizeof(n_pipe) + dataSize);
            memcpy(packetBuffer.data(), &header, sizeof(n_pipe));
            memcpy(packetBuffer.data() + sizeof(n_pipe), data, dataSize);

            DWORD bytesWritten = 0;
            BOOL success = WriteFile(
                hPipe,
                packetBuffer.data(),
                static_cast<DWORD>(packetBuffer.size()),
                &bytesWritten,
                NULL
            );

            _destroyN_pipe_(&hPipe);

            if (!success || bytesWritten != packetBuffer.size()) {
                return AL_WRITE_FAILED;
            }
            return ALGIS_OK;
        }

        // Shared memory
        else if (transportType == XSMEM) {

        }

        return AL_INVALID_PARAMETER;
    }
    ALGIS_STATUS __stdcall _xrecieve_(int transportType, LPCWSTR ipcName, void* buffer, size_t bufferSize, DWORD* bytesRead, DWORD timeoutMs) {
        if (!ipcName || !buffer || bufferSize == 0 || !bytesRead) {
            return AL_INVALID_PARAMETER;
        }

        *bytesRead = 0;

        // named pipes
        if (transportType == XPIPE) {
            HANDLE hPipe = INVALID_HANDLE_VALUE;

            size_t totalExpectedSize = sizeof(n_pipe) + bufferSize;
            ALGIS_STATUS status = _createN_pipe_(ipcName, static_cast<DWORD>(totalExpectedSize), &hPipe);
            if (status != ALGIS_OK) return status;

            BOOL connected = ConnectNamedPipe(hPipe, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);

            if (!connected) {
                _destroyN_pipe_(&hPipe);
                return AL_CONNECT_FAILED;
            }
            
            std::vector<BYTE> readBuffer(totalExpectedSize);
            DWORD dwRead = 0;
            BOOL success = ReadFile(hPipe, readBuffer.data(), static_cast<DWORD>(readBuffer.size()), &dwRead, NULL);

            _destroyN_pipe_(&hPipe);

            if (!success || dwRead < sizeof(n_pipe)) {
                return AL_READ_FAILED;
            }

            n_pipe* header = reinterpret_cast<n_pipe*>(readBuffer.data());
            if (memcmp(header->magic, IPC_MAGIC, sizeof(header->magic)) != 0) {
                return AL_INTERNAL_UNKNOWN_SYSTEM_ERROR;
            }

            DWORD acutalPayloadSize = dwRead - sizeof(n_pipe);
            if (acutalPayloadSize > bufferSize) {
                acutalPayloadSize = static_cast<DWORD>(bufferSize);
            }

            memcpy(buffer, readBuffer.data() + sizeof(n_pipe), acutalPayloadSize);
            *bytesRead = acutalPayloadSize;
            return ALGIS_OK;
        }

        // shared mmeory
        else if (transportType == XSMEM) {

        }

        return AL_INVALID_PARAMETER;
    }

#endif //< @end _OS_WINDOWS


    } // namespace detail

#if defined(_OS_WINDOWS)
#if defined(ALGIS_EXTERNAL)
// @begin external code
/*
    * Initialize this library, call this only ONCE ever, at the beginning of your program.
    *
    * IN - *processName -> Name of the process you want to target.
    * Usage: if (EH_Initialize("RobloxPlayerBeta.exe") != EH_OK) { return; }
    */
    ALGIS_STATUS __stdcall hk_init(const char* processName) {
        return detail::_al_hk_init(processName);
    }

    /*
    * Uninitialize this library, call this only ONCE ever, at the end of your program.
    * Usage: (pretend we are at the end of the file) if (EH_Unitialize() != EH_OK) { return; }
    */
    ALGIS_STATUS __stdcall hk_uninit(VOID) {
        return detail::_al_hk_uninit();
    }

    /*
    * Maps your dll into memory via your target PID
    * IN ->  *DLLPath -> THe absoulte path of your DLL.
    * OUT -> *oEntryPoint -> The address of your entry point. (NEEDED)
    */
    ALGIS_STATUS __stdcall map(const wchar_t* dllPath, uintptr_t* oEntryPoint) {
        return detail::_al_map(dllPath, oEntryPoint);
    }

    /*
    * Creates an external hook via IAT hooking.
    *
    * IN -  *jmpInstruction -> The function you want to hook on.
    * IN -  *newTarget -> Your exported function in your dll (edit mmap.cpp for that).
    * OUT - *outHook -> Stores the hook pointer, to destroy later with EH_CleanupHook.
    *
    *
    * The entry.remoteBase is from my mmaper. If you want to put your own
    * custom mmaper, just comment out EH_Map and remove mmap.h/mmap.cpp.
    * This is old comments and I'm too lazy to replace it, so just piece together the comments.
    *
    * Usage: if (EH_CreateHook(QPC, entry.remoteBase) != EH_OK) { return; }
    * OR
    * uintptr_t entry = 0;
    * if (EH_Map("dllpath.dll", &entry) == EH_OK { EH_CreateHook(targetAddr, (void*)entry); }
    */
    ALGIS_STATUS __stdcall create_hook(void* jmpInstruction, void* newTarget, A_HOOK* oHook) {
        return detail::_a_create_hook(jmpInstruction, newTarget, oHook);
    }

    /*
    * Call this function to clean up a specific hook.
    * If you want to destroy every hook, use EH_DESTROY
    *
    * IN - hook -> Cleans only one specific hook.
    *
    * Usage: EH_STATUS status = EH_CleanHook(qpcHook);
    * OR
    * EH_STATUS status = EH_CleanHook(EH_DESTROY);
    */
    ALGIS_STATUS __stdcall destroy_hook(A_HOOK hook) {
        return detail::_a_destroy_hook(hook);
    }

    /*
    * CreateNPipe
    * Used to create a standard bi-directonal named pipe. (!!FOR THE SERVER!!)
    *
    * IN -> pipeName -> Name of your pipe, MUST START WITH "\\\\.\\pipe\\[your_name]"
    * IN -> bufferSize -> Output buffer size and input buffer size
    * OUT -> *oPipeHandle -> The pipe handle, will be used to destroy your pipe
    */
    ALGIS_STATUS __stdcall createNPipe(LPCWSTR pipeName, DWORD bufferSize, HANDLE* oPipeHandle) {
        return detail::_createN_pipe_(pipeName, bufferSize, oPipeHandle);
    }

    /*
    * OpenNPipe
    * Used to connect/open a existing named pipe. (!!FOR THE CLIENT!!)
    *
    * IN -> pipeName -> Name of your pipe
    * IN -> timeoutMs -> timeout miliseconds
    * OUT -> *oPipeHandle -> The pipe handle, used to send data to the server.
    */
    ALGIS_STATUS __stdcall openNPipe(LPCWSTR pipeName, DWORD timeoutMs, HANDLE* oPipeHandle) {
        return detail::_openN_pipe_(pipeName, timeoutMs, oPipeHandle);
    }

    /*
    * DestroyNPipe
    * Used to destroy a named pipe created with CreateNPipe,
    *
    * IN -> *pipeHandle -> The handle to your pipe.
    */
    ALGIS_STATUS __stdcall destroyNPipe(HANDLE* pipeHandle) {
        return detail::_destroyN_pipe_(pipeHandle);
    }

    /*
    * XSend
    * Sends whatever you want, to whichever IPC
    *
    * IN -> transportType -> You will use two values for this, XPIPE (1) for named pipes and XSMEM (2) for shared memory.
    * IN -> ipcName -> The exact name you put for your type of IPC, the name of your shared pipe or the name for the shared memory. !!BE EXACT!!
    * IN -> *data -> The data you want to send
    * IN -> dataSize -> The size of the data your putting in
    * OPT -> seperateThread -> if you want to create your IPC in a seperate thread, use IPC_THREAD_CREATE
    */
    ALGIS_STATUS __stdcall xsend(int transportType, LPCWSTR ipcName, const void* data, size_t dataSize, bool seperateThread = false) {
        if (!ipcName || !data || dataSize == 0) {
            return AL_INVALID_PARAMETER;
        }

        if (seperateThread) {
            std::vector<BYTE> dataCopy(static_cast<const BYTE*>(data), static_cast<const BYTE*>(data) + dataSize);
            std::wstring nameCopy(ipcName);

            std::thread([transportType, nameCopy, dataCopy]() {
                detail::_xsend_(transportType, nameCopy.c_str(), dataCopy.data(), dataCopy.size());
            }).detach();

            return ALGIS_OK;
        }
        else {
            return detail::_xsend_(transportType, ipcName, data, dataSize);
        }
    }

    /*
    * XRecieve
    * Recieves data from the IPC
    *
    * IN -> transportType -> XPIPE (1) or XSMEM (2)
    * IN -> ipcName -> The exact name of your pipe or shared memory section
    * OUT -> buffer -> Output buffer to store incoming data
    * IN -> bufferSize -> Capacity of output buffer
    * OUT -> bytesRead -> Number of bytes actually read/recieved
    * IN -> timeoutMs -> Time in miliseconds to wait for data before timing out
    * OPT -> seperateThread -> if you want to create your IPC in a seperate thread, use IPC_THREAD_CREATE
    * OPT -> callback -> for async use, an example would be void myCallback(void* data, size_t size) { // define my logic here } and use that as callback
    */
    ALGIS_STATUS __stdcall xrecieve(int transportType, LPCWSTR ipcName, void* oBuffer, size_t bufferSize, DWORD* oBytesRead, DWORD timeoutMs, pFnAlgisCallback callback = nullptr, bool seperateThread = false) {
        if (seperateThread) {
            if (!callback) return AL_INVALID_PARAMETER;

            std::wstring nameCopy(ipcName);

            std::thread([transportType, nameCopy, timeoutMs, callback]() {
                std::vector<BYTE> localBuffer(4096);
                DWORD bytesRead = 0;

                ALGIS_STATUS status = detail::_xrecieve_(transportType, nameCopy.c_str(), localBuffer.data(), localBuffer.size(), &bytesRead, timeoutMs);
                
                if (status == ALGIS_OK && bytesRead > 0) {
                    callback(localBuffer.data(), bytesRead);
                }
            }).detach();

            return ALGIS_OK;
        }
        else {
            return detail::_xrecieve_(transportType, ipcName, oBuffer, bufferSize, oBytesRead, timeoutMs);
        }
    }
#endif // @end external code



#if defined(ALGIS_INTERNAL) //< @begin internal helpers
    // _CRT_INIT (needed for mmappers!)
    extern "C" {
        extern BOOL WINAPI _CRT_INIT(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved);
    }

    ALGIS_STATUS createTP_work(PTP_WORK_CALLBACK callback, PVOID context = NULL) {
        return detail::_create_tp_(callback, context);
    }

    BOOL createVHook(void* detour, void* classInstance, size_t index, void** oOriginal = nullptr) {
        return _createVHook_(detour, classInstance, index, oOriginal);
    }

    BOOL enableVHook(void* detour) {
        return _enableVHook_(detour);
    }

    BOOL removeVHook(void* detour) {
        return _removeVHook_(detour);
    }

    void* getVOrignal(void* detour) {
        return _getVOrignal_(detour);
    }

#endif //< @end internal helpers
#endif //< @end _OS_WINDOWS

} // namespace algis
#endif // ALGIS_H