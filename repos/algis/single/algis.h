#ifndef ALGIS_H
#define ALGIS_H
// !! THIS IS NOW DEPRECATED !!
// !! THIS IS NOW DEPRECATED !!
// !! THIS IS NOW DEPRECATED !!
// !! THIS IS NOW DEPRECATED !!
// !! THIS IS NOW DEPRECATED !!
// !! THIS IS NOW DEPRECATED !!
// !! THIS IS NOW DEPRECATED !!
// !! THIS IS NOW DEPRECATED !!
// !! THIS IS NOW DEPRECATED !!
// !! THIS IS NOW DEPRECATED !!
//        __     _   
//  ___ _/ /__ _(_)__
// / _ `/ / _ `/ (_-<
// \_,_/_/\_, /_/___/
//       /___/          

// algis.h
// a header file filled with all types of stuff for game cheats and such
// a skids dream

// everything will be documented, windows only as im too lazy for linux support
// if your confused, just scroll through the files yeah

// ALGIS_LEAN < cuts out the networking part of the lib
// ALGIS_EXTERNAL < user defined; are you internal, or external?
// ALGIS_INTERNAL < user defined; are you internal, or external?

// !! SINGLE HEADER !!


// system headers
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <ntstatus.h>
#include <winternl.h>
#include <intrin.h>
#include <tlhelp32.h>
#if !defined(ALGIS_LEAN)
#include <wininet.h>
#pragma comment(lib, "wininet.lib")
#endif

// universal headers
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
#include <memory>


// constant defs.
#ifndef XPIPE
#define XPIPE 1 //< used in named pipes
#endif

#ifndef XSMEM
#define XSMEM 2 //< used in shared memory
#endif

#ifndef DESTROY_HOOK_ALL
#define DESTROY_HOOK_ALL nullptr //< used to destroy every hook
#endif

#ifndef IPC_THREAD_CREATE
#define IPC_THREAD_CREATE true //< used to create an IPC in a seperate threaad
#endif

#ifndef IPC_MAGIC
#define IPC_MAGIC "algis_ipc" //< used for auth between IPC's
#endif

typedef void* HOOK; //< shortname for void*
typedef void(*fnAlgisCallback)(void* data, size_t size);

#ifndef NT_SUCCESS
#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)
#endif




#ifndef NTSTATUS
typedef LONG NTSTATUS;
#endif

// just some useful macros
#define __attr_pack__ _Pragma("pack(push, 1)")
#define __attr_pop__  _Pragma("pack(pop)")

#if defined(ALGIS_INTERNAL) && defined(ALGIS_EXTERNAL)
#error "cannot define both ALGIS_INTERNAL and ALGIS_EXTERNAL"
#endif

#if !defined(ALGIS_INTERNAL) && !defined(ALGIS_EXTERNAL)
#define ALGIS_EXTERNAL
#endif

#if !(defined _M_IX86) && !(defined _M_X64) && !(defined __i386__) && !(defined __x86_64__)
#error "X86-64 are the only supported types."
#endif

//< logger class
class FLogger {
private:
	HANDLE MFile;
	CRITICAL_SECTION MCS;

	FLogger() : MFile(INVALID_HANDLE_VALUE) {
		InitializeCriticalSection(&MCS);
	}

	~FLogger() {
		if (MFile != INVALID_HANDLE_VALUE) {
			CloseHandle(MFile);
		}
		DeleteCriticalSection(&MCS);
	}

	FLogger(const FLogger&) = delete;
	FLogger& operator=(const FLogger&) = delete;

public:
	static FLogger& GetLogInstance() {
		static FLogger Instance;
		return Instance;
	}

	bool LogInit(const std::string& filename) {
		EnterCriticalSection(&MCS);
		if (MFile != INVALID_HANDLE_VALUE) {
			CloseHandle(MFile);
		}
		MFile = CreateFileA(
			filename.c_str(),
			FILE_APPEND_DATA,
			FILE_SHARE_READ,
			NULL,
			OPEN_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,
			NULL);
		LeaveCriticalSection(&MCS);
		return MFile != INVALID_HANDLE_VALUE;
	}

	template <typename ... Args>
	void Log(const char* level, const char* fmt, Args... args) {
		EnterCriticalSection(&MCS);
		if (MFile == INVALID_HANDLE_VALUE) {
			LeaveCriticalSection(&MCS);
			return;
		}

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
		std::snprintf(msgBuf, sizeof(msgBuf), fmt, args...);

		char finalBuf[2048];
		int len = std::snprintf(finalBuf, sizeof(finalBuf), "[%s] [%s] %s\n", timeBuf, level, msgBuf);

		if (len > 0) {
			DWORD written = 0;
			WriteFile(MFile, finalBuf, (DWORD)len, &written, NULL);
			FlushFileBuffers(MFile);
		}

		LeaveCriticalSection(&MCS);
	}
};
// log macros 
#define LOG_INIT(filename) FLogger::GetLogInstance().LogInit(filename)
#define ILOG(fmt, ...)     FLogger::GetLogInstance().Log("INFO", fmt, ##__VA_ARGS__)
#define WLOG(fmt, ...)     FLogger::GetLogInstance().Log("WARN", fmt, ##__VA_ARGS__)
#define ELOG(fmt, ...)     FLogger::GetLogInstance().Log("ERROR", fmt, ##__VA_ARGS__)


// main error enum.
// to lazy to update it so yknow
typedef enum _ALGIS_STATUS : std::uint32_t {
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
	AL_INTERNAL_UNKNOWN_SYSTEM_ERROR,

    // just a placeholder status
    ALGIS_NULL_STATUS
} ALGIS_STATUS;

// enum debugger to help
inline const char* al_to_string(ALGIS_STATUS status) {
	switch (status) {
	case ALGIS_OK: return "ALGIS_OK";
	case AL_PROCESS_NOT_FOUND: return "AL_PROCESS_NOT_FOUND";
	case AL_HANDLE_NOT_FOUND: return "AL_HANDLE_NOT_FOUND";
	case AL_INVALID_ADDRESS: return "AL_INVALID_ADDRESS";
	case AL_INVALID_PARAMETER: return "AL_INVALID_PARAMETER";
	case AL_OOM: return "AL_OOM";
	case AL_ALREADY_EXISTS: return "AL_ALREADY_EXISTS";
	case AL_TIMEOUT: return "AL_TIMEOUT";
	case AL_FILE_NOT_FOUND: return "AL_FILE_NOT_FOUND";
	case AL_ERR_MAPPING_FAILED: return "AL_ERR_MAPPING_FAILED";
	case AL_HOOK_FAILED: return "AL_HOOK_FAILED";
	case AL_MAPPING_CREATE_FAILED: return "AL_MAPPING_CREATE_FAILED";
	case AL_MAPPING_OPEN_FAILED: return "AL_MAPPING_OPEN_FAILED";
	case AL_MAP_VIEW_FAILED: return "AL_MAP_VIEW_FAILED";
	case AL_UNMAP_VIEW_FAILED: return "AL_UNMAP_VIEW_FAILED";
	case AL_WAIT_TIMEOUT: return "AL_WAIT_TIMEOUT";
	case AL_CREATE_FAILED: return "AL_CREATE_FAILED";
	case AL_CONNECT_FAILED: return "AL_CONNECT_FAILED";
	case AL_BUSY: return "AL_BUSY";
	case AL_BROKEN: return "AL_BROKEN";
	case AL_DISCONNECTED: return "AL_DISCONNECTED";
	case AL_MORE_DATA_AVAILABLE: return "AL_MORE_DATA_AVAILABLE";
	case AL_MODE_CHANGE_FAILED: return "AL_MODE_CHANGE_FAILED";
	case AL_WRITE_FAILED: return "AL_WRITE_FAILED";
	case AL_READ_FAILED: return "AL_READ_FAILED";
	case AL_INTERNAL_UNKNOWN_SYSTEM_ERROR: return "AL_INTERNAL_UNKNOWN_SYSTEM_ERROR";
	default: return "UNKNOWN_ALGIS_STATUS";
	}
}


// main ipc header
__attr_pack__ 
typedef struct _ipc_header {
	char magic[10]; // stores "algis_ipc"
	std::uint16_t transport; // XPIPE or XSMEM
	std::uint32_t PayloadSize;
} ipc_header_t;
__attr_pop__

// ipc cmds
__attr_pack__
typedef struct _ipc_cmd {
	std::uint16_t cmd_id;
	std::uint16_t flags;
	std::uint64_t addr;
	std::uint32_t size;
	std::uint32_t reserved;
} ipc_cmd_t;
__attr_pop__

// ipc reserve
__attr_pack__
typedef struct _ipc_res {
	std::uint16_t cmd_id;
	ALGIS_STATUS  status;
	std::uint32_t size;
} ipc_res_t;
__attr_pop__

__attr_pack__
typedef enum _ALGIS_CMD : std::uint16_t {
	AL_PING    = 0x01, // alive check, no payload
	AL_DATA    = 0x02, // send data blob 
	AL_EXECUTE = 0x03, // execute a function/address
} ALGIS_CMD;
__attr_pop__


// the main namespace, where most of the functions are at
//> @begin algis
namespace algis {

	// the code in detail is SUPER messy, barely readable and more!
	// not sure what you want to do
	//> @begin detail (internal)
	namespace detail {
		/////////////////////////////////////////////////////////////////
		////////////////// !! START OF NETWORKING LIB !! ////////////////
		/////////////////////////////////////////////////////////////////
#if !defined(ALGIS_LEAN) //< @begin ALGIS_LEAN
		// networking stuff later
#endif //< @end ALGIS_LEAN
		/////////////////////////////////////////////////////////////////
		////////////////// !! START OF EXT/INT IMPL !! //////////////////
		/////////////////////////////////////////////////////////////////
#if defined(ALGIS_EXTERNAL) //< @begin ALGIS_EXTERNAL
		typedef enum _SECTION_INHERIT {
			ViewShare = 1,
			ViewUnmap = 2
		} SECTION_INHERIT;
		// syscall resovler
// SYSTEM_INFORMATION_CLASS — partial, just what you need
		typedef enum _SYSTEM_INFORMATION_CLASS {
			SystemBasicInformation = 0,
			SystemHandleInformation = 16,
			SystemProcessInformation = 5,
			SystemModuleInformation = 11,
		} SYSTEM_INFORMATION_CLASS;

		// per-handle entry
		typedef struct _SYSTEM_HANDLE {
			ULONG       ProcessId;
			UCHAR       ObjectTypeNumber;
			UCHAR       Flags;
			USHORT      Handle;
			PVOID       Object;
			ACCESS_MASK GrantedAccess;
		} SYSTEM_HANDLE, * PSYSTEM_HANDLE;

		// top-level buffer layout
		typedef struct _SYSTEM_HANDLE_INFORMATION {
			ULONG         HandleCount;
			SYSTEM_HANDLE Handles[1];
		} SYSTEM_HANDLE_INFORMATION, * PSYSTEM_HANDLE_INFORMATION;

		// needed for NtOpenProcess
		typedef struct _CLIENT_ID {
			HANDLE UniqueProcess;
			HANDLE UniqueThread;
		} CLIENT_ID, * PCLIENT_ID;

		// needed for NtOpenProcess / NtDuplicateObject
		typedef struct _OBJECT_ATTRIBUTES {
			ULONG           Length;
			HANDLE          RootDirectory;
			PUNICODE_STRING ObjectName;
			ULONG           Attributes;
			PVOID           SecurityDescriptor;
			PVOID           SecurityQualityOfService;
		} OBJECT_ATTRIBUTES, * POBJECT_ATTRIBUTES;
		namespace syscall {
			// forward decl
			template <typename Fn, typename... Args>
			inline NTSTATUS _indirect(std::uint32_t ssn, Args... args);

			struct SEntry {
				std::uint32_t ssn = 0;
				bool resolved = false;
			}; // struct SEntry

			typedef struct _CLIENT_ID {
				HANDLE UniqueProcess;
				HANDLE UniqueThread;
			} CLIENT_ID, * PCLIENT_ID; // PCLIENT_ID


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
					return ptr[0] == 0x4C &&
						ptr[1] == 0x8B &&
						ptr[2] == 0xD1 &&
						ptr[3] == 0xB8;
				}

				inline std::uint32_t ExtractSSN(std::uint8_t* ptr) {
					return *reinterpret_cast<std::uint32_t*>(ptr + 4);
				}

				//< halo gates resolver
				inline std::uint32_t ResolveSSN(const char* name) {
					HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
					if (!ntdll) return 0;

					std::uint8_t* base = reinterpret_cast<uint8_t*>(ntdll);

					IMAGE_DOS_HEADER* dos = reinterpret_cast<IMAGE_DOS_HEADER*>(base);
					IMAGE_NT_HEADERS* nt = reinterpret_cast<IMAGE_NT_HEADERS*>(base + dos->e_lfanew);
					IMAGE_EXPORT_DIRECTORY* exp = reinterpret_cast<IMAGE_EXPORT_DIRECTORY*>(
						base + nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);

					DWORD* names = reinterpret_cast<DWORD*>(base + exp->AddressOfNames);
					DWORD* funcs = reinterpret_cast<DWORD*>(base + exp->AddressOfFunctions);
					WORD* ords = reinterpret_cast<WORD*> (base + exp->AddressOfNameOrdinals);

					std::int32_t target_idx = -1;
					for (DWORD i = 0; i < exp->NumberOfNames; i++) {
						if (strcmp(reinterpret_cast<const char*>(base + names[i]), name) == 0) {
							target_idx = static_cast<std::int32_t>(i);
							break;
						}
					}

					if (target_idx < 0) return 0;

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
					if (!ntdll) return nullptr;

					std::uint8_t* base = reinterpret_cast<std::uint8_t*>(ntdll);
					IMAGE_DOS_HEADER* dos = reinterpret_cast<IMAGE_DOS_HEADER*>(base);
					IMAGE_NT_HEADERS* nt = reinterpret_cast<IMAGE_NT_HEADERS*>(base + dos->e_lfanew);

					IMAGE_SECTION_HEADER* sec = IMAGE_FIRST_SECTION(nt);
					for (WORD i = 0; i < nt->FileHeader.NumberOfSections; i++, sec++) {
						if (std::memcmp(sec->Name, ".text", 5) != 0) continue;

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
						_stub_page = reinterpret_cast<uint8_t*>(
							VirtualAlloc(nullptr, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));
						if (!_stub_page) return nullptr;
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

			} //< namespace internal_ssn

			// indirect dispatch
			template <typename Fn, typename... Args>
			inline NTSTATUS _indirect(std::uint32_t ssn, Args... args) {
				std::uint8_t* stub = internal_ssn::GetOrBuildStub(ssn);
				if (!stub) return STATUS_UNSUCCESSFUL;
				return reinterpret_cast<Fn>(stub)(args...);
			}

			// init, call before using wrappers
			inline bool Init() {
				if (internal_ssn::_ready) return true;
				internal_ssn::_gadget = internal_ssn::FindGadget();
				if (!internal_ssn::_gadget) return false;

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
			inline NTSTATUS AllocateVirtualMemory(
				HANDLE      ProcessHandle,
				PVOID* BaseAddress,
				ULONG_PTR   ZeroBits,
				PSIZE_T     RegionSize,
				ULONG       AllocationType,
				ULONG       Protect)
			{
				using Fn = NTSTATUS(NTAPI*)(HANDLE, PVOID*, ULONG_PTR, PSIZE_T, ULONG, ULONG);
				return _indirect<Fn>(SSN::NtAllocateVirtualMemory.ssn,
					ProcessHandle, BaseAddress, ZeroBits, RegionSize, AllocationType, Protect);
			}

			inline NTSTATUS FreeVirtualMemory(
				HANDLE  ProcessHandle,
				PVOID* BaseAddress,
				PSIZE_T RegionSize,
				ULONG   FreeType)
			{
				using Fn = NTSTATUS(NTAPI*)(HANDLE, PVOID*, PSIZE_T, ULONG);
				return _indirect<Fn>(SSN::NtFreeVirtualMemory.ssn,
					ProcessHandle, BaseAddress, RegionSize, FreeType);
			}

			inline NTSTATUS ProtectVirtualMemory(
				HANDLE  ProcessHandle,
				PVOID* BaseAddress,
				PSIZE_T RegionSize,
				ULONG   NewProtect,
				PULONG  OldProtect)
			{
				using Fn = NTSTATUS(NTAPI*)(HANDLE, PVOID*, PSIZE_T, ULONG, PULONG);
				return _indirect<Fn>(SSN::NtProtectVirtualMemory.ssn,
					ProcessHandle, BaseAddress, RegionSize, NewProtect, OldProtect);
			}
			inline NTSTATUS CreateSection(
				PHANDLE            SectionHandle,
				ACCESS_MASK        DesiredAccess,
				POBJECT_ATTRIBUTES ObjectAttributes,
				PLARGE_INTEGER     MaximumSize,
				ULONG              SectionPageProtection,
				ULONG              AllocationAttributes,
				HANDLE             FileHandle)
			{
				using Fn = NTSTATUS(NTAPI*)(PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES, PLARGE_INTEGER, ULONG, ULONG, HANDLE);
				return _indirect<Fn>(SSN::NtCreateSection.ssn,
					SectionHandle, DesiredAccess, ObjectAttributes,
					MaximumSize, SectionPageProtection, AllocationAttributes, FileHandle);
			}

			inline NTSTATUS MapViewOfSection(
				HANDLE          SectionHandle,
				HANDLE          ProcessHandle,
				PVOID* BaseAddress,
				ULONG_PTR       ZeroBits,
				SIZE_T          CommitSize,
				PLARGE_INTEGER  SectionOffset,
				PSIZE_T         ViewSize,
				DWORD           InheritDisposition,
				ULONG           AllocationType,
				ULONG           Win32Protect)
			{
				using Fn = NTSTATUS(NTAPI*)(HANDLE, HANDLE, PVOID*, ULONG_PTR, SIZE_T, PLARGE_INTEGER, PSIZE_T, DWORD, ULONG, ULONG);
				return _indirect<Fn>(SSN::NtMapViewOfSection.ssn,
					SectionHandle, ProcessHandle, BaseAddress,
					ZeroBits, CommitSize, SectionOffset, ViewSize,
					InheritDisposition, AllocationType, Win32Protect);
			}
			inline NTSTATUS Close(
				HANDLE ProcessHandle
			)
			{
				using Fn = NTSTATUS(NTAPI*)(HANDLE);
				return _indirect<Fn>(SSN::NtClose.ssn, ProcessHandle);
			}
			inline NTSTATUS OpenProcess(
				PHANDLE             ProcessHandle,
				ACCESS_MASK         DesiredAccess,
				POBJECT_ATTRIBUTES  ObjectAttributes,
				PCLIENT_ID          ClientId)
			{
				using Fn = NTSTATUS(NTAPI*)(PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES, PCLIENT_ID);
				return _indirect<Fn>(SSN::NtOpenProcess.ssn,
					ProcessHandle, DesiredAccess, ObjectAttributes, ClientId);
			}
			inline NTSTATUS WriteVirtualMemory(
				HANDLE  ProcessHandle,
				PVOID   BaseAddress,
				PVOID   Buffer,
				SIZE_T  NumberOfBytesToWrite,
				PSIZE_T NumberOfBytesWritten)
			{
				using Fn = NTSTATUS(NTAPI*)(HANDLE, PVOID, PVOID, SIZE_T, PSIZE_T);
				return _indirect<Fn>(SSN::NtWriteVirtualMemory.ssn,
					ProcessHandle, BaseAddress, Buffer, NumberOfBytesToWrite, NumberOfBytesWritten);
			}

			inline NTSTATUS ReadVirtualMemory(
				HANDLE  ProcessHandle,
				PVOID   BaseAddress,
				PVOID   Buffer,
				SIZE_T  NumberOfBytesToRead,
				PSIZE_T NumberOfBytesRead)
			{
				using Fn = NTSTATUS(NTAPI*)(HANDLE, PVOID, PVOID, SIZE_T, PSIZE_T);
				return _indirect<Fn>(SSN::NtReadVirtualMemory.ssn,
					ProcessHandle, BaseAddress, Buffer, NumberOfBytesToRead, NumberOfBytesRead);
			}

			inline NTSTATUS QueryVirtualMemory(
				HANDLE                    ProcessHandle,
				PVOID                     BaseAddress,
				DWORD  MemoryInformationClass,
				PVOID                     MemoryInformation,
				SIZE_T                    MemoryInformationLength,
				PSIZE_T                   ReturnLength)
			{
				using Fn = NTSTATUS(NTAPI*)(HANDLE, PVOID, DWORD, PVOID, SIZE_T, PSIZE_T);
				return _indirect<Fn>(SSN::NtQueryVirtualMemory.ssn,
					ProcessHandle, BaseAddress, MemoryInformationClass,
					MemoryInformation, MemoryInformationLength, ReturnLength);
			}

			inline NTSTATUS SuspendThread(
				HANDLE  ThreadHandle,
				PULONG  PreviousSuspendCount = nullptr)
			{
				using Fn = NTSTATUS(NTAPI*)(HANDLE, PULONG);
				return _indirect<Fn>(SSN::NtSuspendThread.ssn, ThreadHandle, PreviousSuspendCount);
			}

			inline NTSTATUS ResumeThread(
				HANDLE  ThreadHandle,
				PULONG  PreviousSuspendCount = nullptr)
			{
				using Fn = NTSTATUS(NTAPI*)(HANDLE, PULONG);
				return _indirect<Fn>(SSN::NtResumeThread.ssn, ThreadHandle, PreviousSuspendCount);
			}

			inline NTSTATUS GetContextThread(
				HANDLE   ThreadHandle,
				PCONTEXT ThreadContext)
			{
				using Fn = NTSTATUS(NTAPI*)(HANDLE, PCONTEXT);
				return _indirect<Fn>(SSN::NtGetContextThread.ssn, ThreadHandle, ThreadContext);
			}

			inline NTSTATUS SetContextThread(
				HANDLE   ThreadHandle,
				PCONTEXT ThreadContext)
			{
				using Fn = NTSTATUS(NTAPI*)(HANDLE, PCONTEXT);
				return _indirect<Fn>(SSN::NtSetContextThread.ssn, ThreadHandle, ThreadContext);
			}


			inline NTSTATUS QuerySystemInformation(
				SYSTEM_INFORMATION_CLASS SystemInformationClass,
				PVOID                    SystemInformation,
				ULONG                    SystemInformationLength,
				PULONG                   ReturnLength)
			{
				using Fn = NTSTATUS(NTAPI*)(SYSTEM_INFORMATION_CLASS, PVOID, ULONG, PULONG);
				return _indirect<Fn>(SSN::NtQuerySystemInformation.ssn,
					SystemInformationClass, SystemInformation, SystemInformationLength, ReturnLength);
			}

			inline NTSTATUS QueryInformationProcess(
				HANDLE           ProcessHandle,
				PROCESSINFOCLASS ProcessInformationClass,
				PVOID            ProcessInformation,
				ULONG            ProcessInformationLength,
				PULONG           ReturnLength)
			{
				using Fn = NTSTATUS(NTAPI*)(HANDLE, PROCESSINFOCLASS, PVOID, ULONG, PULONG);
				return _indirect<Fn>(SSN::NtQueryInformationProcess.ssn,
					ProcessHandle, ProcessInformationClass,
					ProcessInformation, ProcessInformationLength, ReturnLength);
			}


			inline NTSTATUS DuplicateObject(
				HANDLE      SourceProcessHandle,
				HANDLE      SourceHandle,
				HANDLE      TargetProcessHandle,
				PHANDLE     TargetHandle,
				ACCESS_MASK DesiredAccess,
				ULONG       HandleAttributes,
				ULONG       Options)
			{
				using Fn = NTSTATUS(NTAPI*)(HANDLE, HANDLE, HANDLE, PHANDLE, ACCESS_MASK, ULONG, ULONG);
				return _indirect<Fn>(SSN::NtDuplicateObject.ssn,
					SourceProcessHandle, SourceHandle, TargetProcessHandle,
					TargetHandle, DesiredAccess, HandleAttributes, Options);
			}

			inline NTSTATUS QueryObject(
				HANDLE                   Handle,
				OBJECT_INFORMATION_CLASS ObjectInformationClass,
				PVOID                    ObjectInformation,
				ULONG                    ObjectInformationLength,
				PULONG                   ReturnLength)
			{
				using Fn = NTSTATUS(NTAPI*)(HANDLE, OBJECT_INFORMATION_CLASS, PVOID, ULONG, PULONG);
				return _indirect<Fn>(SSN::NtQueryObject.ssn,
					Handle, ObjectInformationClass,
					ObjectInformation, ObjectInformationLength, ReturnLength);
			}
			inline NTSTATUS UnmapViewOfSection(
				HANDLE ProcessHandle,
				PVOID  BaseAddress)
			{
				using Fn = NTSTATUS(NTAPI*)(HANDLE, PVOID);
				return _indirect<Fn>(SSN::NtUnmapViewOfSection.ssn,
					ProcessHandle, BaseAddress);
			}

			inline NTSTATUS UnmapViewOfSectionEx(
				HANDLE ProcessHandle,
				PVOID  BaseAddress,
				ULONG  Flags)
			{
				using Fn = NTSTATUS(NTAPI*)(HANDLE, PVOID, ULONG);
				return _indirect<Fn>(SSN::NtUnmapViewOfSectionEx.ssn,
					ProcessHandle, BaseAddress, Flags);
			}

		} // namespace syscall

		class Process {
		private:
			std::string name;
			DWORD pid;
			HANDLE handle;

			HANDLE hijack_handle(DWORD TargetPID);
			DWORD GetPID(std::string targetName);

		public:
			Process(std::string targetName);
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
					if (handle) CloseHandle(handle);
					name = std::move(other.name);
					pid = other.pid;
					handle = other.handle;   // ← take it
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

			void SuspendAllThreads(DWORD PID, bool suspend);
		};

		// function defs of the Process class
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
			HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
			typedef NTSTATUS(NTAPI* _RtlAdjustPrivilege)(ULONG, BOOLEAN, BOOLEAN, PBOOLEAN);
			auto RtlAdjustPrivilege = reinterpret_cast<_RtlAdjustPrivilege>(
				GetProcAddress(ntdll, "RtlAdjustPrivilege"));

			BOOLEAN oldPriv;
			if (RtlAdjustPrivilege)
				RtlAdjustPrivilege(20, TRUE, FALSE, &oldPriv);

			NTSTATUS status = 0;
			SIZE_T   size = 0x10000;
			PVOID    buf = nullptr;

			status = syscall::AllocateVirtualMemory(
				GetCurrentProcess(), &buf, 0, &size,
				MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

			if (!NT_SUCCESS(status) || !buf)
				return nullptr;

			while ((status = syscall::QuerySystemInformation(
				SystemHandleInformation,
				buf, static_cast<ULONG>(size),
				reinterpret_cast<PULONG>(&size))) == STATUS_INFO_LENGTH_MISMATCH)
			{
				SIZE_T zero = 0;
				syscall::FreeVirtualMemory(GetCurrentProcess(), &buf, &zero, MEM_RELEASE);
				buf = nullptr;
				size *= 2;

				status = syscall::AllocateVirtualMemory(
					GetCurrentProcess(), &buf, 0, &size,
					MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

				if (!NT_SUCCESS(status) || !buf)
					return nullptr;
			}

			if (!NT_SUCCESS(status)) {
				SIZE_T zero = 0;
				syscall::FreeVirtualMemory(GetCurrentProcess(), &buf, &zero, MEM_RELEASE);
				return nullptr;
			}

			auto* hInfo = reinterpret_cast<PSYSTEM_HANDLE_INFORMATION>(buf);
			HANDLE result = nullptr;

			for (ULONG i = 0; i < hInfo->HandleCount; i++) {
				SYSTEM_HANDLE& sh = hInfo->Handles[i];
				if (!sh.Handle) continue;

				OBJECT_ATTRIBUTES objAttr = { sizeof(OBJECT_ATTRIBUTES) };
				CLIENT_ID         cid = { reinterpret_cast<HANDLE>((ULONG_PTR)sh.ProcessId), nullptr };
				HANDLE            owner = nullptr;

				status = syscall::OpenProcess(&owner, PROCESS_DUP_HANDLE, &objAttr,
					reinterpret_cast<syscall::PCLIENT_ID>(&cid));
				if (!NT_SUCCESS(status) || !owner) continue;

				HANDLE dup = nullptr;
				status = syscall::DuplicateObject(
					owner,
					reinterpret_cast<HANDLE>((ULONG_PTR)sh.Handle),
					GetCurrentProcess(),
					&dup,
					PROCESS_ALL_ACCESS, 0, 0);

				syscall::Close(owner);

				if (NT_SUCCESS(status) && dup) {
					if (GetProcessId(dup) == targetPid) {
						result = dup;
						break;
					}
					syscall::Close(dup);
				}
			}

			SIZE_T zero = 0;
			syscall::FreeVirtualMemory(GetCurrentProcess(), &buf, &zero, MEM_RELEASE);
			return result;
		}

		inline Process::Process(std::string targetName) {
			syscall::Init();
			this->name = targetName;
			this->pid = GetPID(targetName);
			this->handle = hijack_handle(this->pid);

			if (!this->handle) {
				std::cerr << "failed to hijack handle for: " << targetName << std::endl;
			}
		}

		inline Process::~Process() {
			if (handle) syscall::Close(handle);
		}

		inline NTSTATUS Process::read_mem(PVOID base, PVOID buf, SIZE_T size, PSIZE_T read) {
			return syscall::ReadVirtualMemory ? syscall::ReadVirtualMemory(this->handle, base, buf, size, read) : STATUS_INVALID_HANDLE;
		}
		inline NTSTATUS Process::write_mem(PVOID base, PVOID buf, SIZE_T size, PSIZE_T written) {
			return syscall::WriteVirtualMemory ? syscall::WriteVirtualMemory(this->handle, base, buf, size, written) : STATUS_INVALID_HANDLE;
		}
		inline NTSTATUS Process::change_protection(PVOID base, SIZE_T size, ULONG prot, PULONG oldProt) {
			PVOID addressToProtect = base;
			SIZE_T regionSize = size;
			return syscall::ProtectVirtualMemory ? syscall::ProtectVirtualMemory(this->handle, &addressToProtect, &regionSize, prot, oldProt) : STATUS_INVALID_HANDLE;
		}
		inline PVOID Process::allocate_mem(SIZE_T size, ULONG type, ULONG prot) {
			PVOID base = nullptr;
			SIZE_T regionSize = size;
			if (syscall::AllocateVirtualMemory) {
				syscall::AllocateVirtualMemory(this->handle, &base, 0, &regionSize, type, prot);
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
		class mapper {
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
		inline mapper::mapper(Process& process) : process(process) {}
		// fix relocations
		inline bool mapper::fix_relocations(PVOID localBase, uintptr_t remoteBase) {
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
		inline bool mapper::resolve_imports(PVOID localBase) {
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
		inline bool mapper::register_seh(PVOID localBase, uintptr_t remoteBase, SIZE_T imageSize) {
			auto dosHeader = (PIMAGE_DOS_HEADER)localBase;
			auto ntHeaders = (PIMAGE_NT_HEADERS)((uintptr_t)localBase + dosHeader->e_lfanew);

			auto exceptionDir = &ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION];
			if (exceptionDir->VirtualAddress == 0) return true;

			auto funcTable = (PRUNTIME_FUNCTION)((uintptr_t)localBase + exceptionDir->VirtualAddress);
			DWORD entryCount = exceptionDir->Size / sizeof(RUNTIME_FUNCTION);

			return RtlAddFunctionTable(funcTable, entryCount, remoteBase);
		}
		// main mapper function
		inline mapper::MapResult mapper::map(const std::wstring& dllPath) {
			MapResult result = { 0, 0, false };
			auto _NtCreateSection = syscall::CreateSection;
			auto _NtMapViewOfSection = syscall::MapViewOfSection;
			auto _NtUnmapViewOfSection = syscall::UnmapViewOfSection;

			// Safety check
			if (!syscall::SSN::NtCreateSection.resolved ||
				!syscall::SSN::NtMapViewOfSection.resolved ||
				!syscall::SSN::NtUnmapViewOfSection.resolved) return result;

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
			status = _NtMapViewOfSection(section, GetCurrentProcess(), &localBase, 0, imageSize, nullptr, &viewSize, ViewUnmap, 0, PAGE_EXECUTE_READWRITE);
			if (!NT_SUCCESS(status)) { CloseHandle(section); return result; }

			// map remotely
			PVOID remoteBase = nullptr;
			viewSize = imageSize;
			status = _NtMapViewOfSection(section, process.get_handle(), &remoteBase, 0, imageSize, nullptr, &viewSize, ViewUnmap, 0, PAGE_EXECUTE_READWRITE);
			if (!NT_SUCCESS(status)) { _NtUnmapViewOfSection(GetCurrentProcess(), localBase); CloseHandle(section); return result; }

			// copy headers
			memcpy(localBase, dllBuffer.data(), ntHeaders->OptionalHeader.SizeOfHeaders);

			// copy sections
			auto section_header = IMAGE_FIRST_SECTION(ntHeaders);
			for (WORD i = 0; i < ntHeaders->FileHeader.NumberOfSections; i++) {
				if (section_header[i].SizeOfRawData == 0) continue;
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
			void** targetSlot;
			void* orig;

			// hook
			bool hook(Process& proc, void* jmpInstruction, void* newTarget) {
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
		
		// internal defs
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

		inline ALGIS_STATUS __stdcall _a_create_hook(void* jmpInstruction, void* newTarget, HOOK* oHook) {
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

#if defined(ALGIS_INTERNAL)
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
	class internal_memory_t final {
	public:
		inline std::uintptr_t GetModuleBase() {
			return (std::uintptr_t)GetModuleHandleA(NULL);
		}

		template <typename Fn>
		inline Fn Read(std::uintptr_t address) {
			Fn Value{};
			safe_read(address, &Value, sizeof(Fn));
			return Value;
		}

		template <typename Fn>
		inline bool Write(std::uintptr_t address, Fn value) {
			return safe_write(address, &value, sizeof(Fn));
		}
	};
#endif //< @end ALGIS_INTERNAL
	class memory_t final {
	public:
		memory_t() = default;
		~memory_t() = default;

		static constexpr bool IsValid(std::uint64_t Address) {
			return Address >= 0x10000 && Address <= 0x7FFFFFFEFFFF;
		}

		std::uint32_t FindProcessId(const char* processName);
		std::uint64_t FindModuleAddress(const std::string& module_name);

		bool AttachToProcess();
		template <typename T>
		T Read(std::uint64_t address);

		template <typename T>
		void Write(std::uint64_t address, T value);

		std::uint32_t FetchProcessId();
		std::uint64_t GetModuleAddress();
		HANDLE GetProcessHandle();
	private:
		std::uint32_t ProcessID;
		std::uint64_t BaseAddress;
		HANDLE ProcessHandle;
	};

	inline std::uint32_t memory_t::FindProcessId(const char* process_name) {
		DWORD pid = 0;

		HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (hSnapshot == INVALID_HANDLE_VALUE) return 0;

		PROCESSENTRY32 PE32;
		PE32.dwSize = sizeof(PROCESSENTRY32);

		if (Process32First(hSnapshot, &PE32)) {
			do
			{
				if (std::strcmp(PE32.szExeFile, process_name) == 0) {
					pid = PE32.th32ProcessID;
					break;
				}
			} while (Process32Next(hSnapshot, &PE32));
		}
		CloseHandle(hSnapshot);
		return this->ProcessID = pid;
	}

	inline bool memory_t::AttachToProcess() {
		HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
		typedef NTSTATUS(NTAPI* _RtlAdjustPrivilege)(ULONG, BOOLEAN, BOOLEAN, PBOOLEAN);
		auto RtlAdjustPrivilege = reinterpret_cast<_RtlAdjustPrivilege>(
			GetProcAddress(ntdll, "RtlAdjustPrivilege"));

		BOOLEAN oldPriv;
		if (RtlAdjustPrivilege)
			RtlAdjustPrivilege(20, TRUE, FALSE, &oldPriv);

		NTSTATUS status = 0;
		SIZE_T   size = 0x10000;
		PVOID    buf = nullptr;

		status = syscall::AllocateVirtualMemory(
			GetCurrentProcess(), &buf, 0, &size,
			MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

		if (!NT_SUCCESS(status) || !buf)
			return false;

		while ((status = syscall::QuerySystemInformation(
			SystemHandleInformation,
			buf, static_cast<ULONG>(size),
			reinterpret_cast<PULONG>(&size))) == STATUS_INFO_LENGTH_MISMATCH)
		{
			SIZE_T zero = 0;
			syscall::FreeVirtualMemory(GetCurrentProcess(), &buf, &zero, MEM_RELEASE);
			buf = nullptr;
			size *= 2;

			status = syscall::AllocateVirtualMemory(
				GetCurrentProcess(), &buf, 0, &size,
				MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

			if (!NT_SUCCESS(status) || !buf)
				return false;
		}

		if (!NT_SUCCESS(status)) {
			SIZE_T zero = 0;
			syscall::FreeVirtualMemory(GetCurrentProcess(), &buf, &zero, MEM_RELEASE);
			return false;
		}

		auto* hInfo = reinterpret_cast<PSYSTEM_HANDLE_INFORMATION>(buf);
		HANDLE result = nullptr;

		for (ULONG i = 0; i < hInfo->HandleCount; i++) {
			SYSTEM_HANDLE& sh = hInfo->Handles[i];
			if (!sh.Handle) continue;

			OBJECT_ATTRIBUTES objAttr = { sizeof(OBJECT_ATTRIBUTES) };
			CLIENT_ID         cid = { reinterpret_cast<HANDLE>((ULONG_PTR)sh.ProcessId), nullptr };
			HANDLE            owner = nullptr;

			status = syscall::OpenProcess(&owner, PROCESS_DUP_HANDLE, &objAttr,
				reinterpret_cast<syscall::PCLIENT_ID>(&cid));
			if (!NT_SUCCESS(status) || !owner) continue;

			HANDLE dup = nullptr;
			status = syscall::DuplicateObject(
				owner,
				reinterpret_cast<HANDLE>((ULONG_PTR)sh.Handle),
				GetCurrentProcess(),
				&dup,
				PROCESS_ALL_ACCESS, 0, 0);

			syscall::Close(owner);

			if (NT_SUCCESS(status) && dup) {
				if (GetProcessId(dup) == this->ProcessID) {
					result = dup;
					break;
				}
				syscall::Close(dup);
			}
		}

		SIZE_T zero = 0;
		syscall::FreeVirtualMemory(GetCurrentProcess(), &buf, &zero, MEM_RELEASE);
		return this->ProcessHandle = result;
	}

	template <typename T>
	T memory_t::Read(std::uint64_t address) {
		T buffer{};

		syscall::ReadVirtualMemory(this->ProcessHandle, reinterpret_cast<HOOK>(address), &buffer, sizeof(T), nullptr);
		return buffer;
	}
	
	template <typename T>
	void memory_t::Write(std::uint64_t address, T value) {
		SIZE_T bytesWritten;
		return syscall::WriteVirtualMemory(this->ProcessHandle, (LPVOID)address, &value, sizeof(T), &bytesWritten);
	}

#endif //< @end ALGIS_EXTERNAL

		/////////////////////////////////////////////////////////////////
		////////////////// !! START OF IPC COMMUNCATION !! //////////////
		///////////////////////////////////////////////////////////////// //< @begin ALGIS_IPC
    namespace {
        bool IsValidIPCName(LPCWSTR name) {
            return name != nullptr && name[0] != L'\0';
        }
    }
    inline ALGIS_STATUS __stdcall _createN_pipe_(LPCWSTR pipeName, DWORD bufferSize, HANDLE* oHPipe) {
        if (!IsValidIPCName(pipeName) || !oHPipe) {
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
            std::wcerr << L"CreateNamedPipe failed, error: " << GetLastError() << '\n';
            return AL_CREATE_FAILED;
        }

        *oHPipe = hPipe;
        return ALGIS_OK;
    }

    inline ALGIS_STATUS __stdcall _openN_pipe_(LPCWSTR pipeName, DWORD timeoutMs, HANDLE* oHPipe) {
        if (!IsValidIPCName(pipeName) || !oHPipe) return AL_INVALID_PARAMETER;

        *oHPipe = INVALID_HANDLE_VALUE;

        if (!WaitNamedPipeW(pipeName, timeoutMs)) {
            std::wcerr << L"WaitNamedPipeW timed out/failed error: " << GetLastError() << '\n';
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
            std::wcerr << L"CreateFileW failed for pipe, error: " << GetLastError() << '\n';
            return AL_CONNECT_FAILED;
        }

        DWORD mode = PIPE_READMODE_MESSAGE;
        if (!SetNamedPipeHandleState(hPipe, &mode, NULL, NULL)) {
            std::wcerr << L"SetNamedPipeHandleState failed, error: " << GetLastError() << '\n';
            CloseHandle(hPipe);
            return AL_MODE_CHANGE_FAILED;
        }

        *oHPipe = hPipe;
        return ALGIS_OK;
    }
    inline ALGIS_STATUS __stdcall _destroyN_pipe_(HANDLE* pipeHandle) {
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

    inline ALGIS_STATUS __stdcall _createSMem_(LPCWSTR memName, int bufferSize, HANDLE *oHandle, void** oBuffer) {
        if (!memName || bufferSize <= 0 || !oHandle || !oBuffer) {
            return AL_INVALID_PARAMETER;
        }

        *oHandle = NULL;
        *oBuffer = NULL;

        HANDLE hMap = CreateFileMappingW(
            INVALID_HANDLE_VALUE,
            NULL,
            PAGE_READWRITE,
            0,
            static_cast<DWORD>(bufferSize),
            memName
        );

        if (hMap == NULL) {
            return AL_MAPPING_CREATE_FAILED;
        }

        void* pBuf = MapViewOfFile(
            hMap,
            FILE_MAP_ALL_ACCESS,
            0, 0, 
            bufferSize
        );

        *oHandle = hMap;
        *oBuffer = pBuf;
        return ALGIS_OK;
    }

    inline ALGIS_STATUS __stdcall _openSMem_(LPCWSTR memName, int bufferSize, HANDLE *oHandle, void **oBuffer) {
        if (!memName || bufferSize <= 0 || !oHandle || !oBuffer) {
            return AL_INVALID_PARAMETER;
        }

        *oHandle = NULL;
        *oBuffer = NULL;

        HANDLE hMap = OpenFileMappingW(
            FILE_MAP_ALL_ACCESS,
            FALSE,
            memName
        );

        if (hMap == NULL) {
            return AL_MAPPING_OPEN_FAILED;
        }

        void* pBuf = MapViewOfFile(
            hMap,
            FILE_MAP_ALL_ACCESS,
            0, 0,
            bufferSize
        );

        if (pBuf == NULL) {
            return AL_MAP_VIEW_FAILED;
        }

        *oHandle = hMap;
        *oBuffer = pBuf;
        return ALGIS_OK;
    }

    inline ALGIS_STATUS __stdcall _destroySMem_(HANDLE mHandle, void* buffer) {
        if (!mHandle || mHandle == INVALID_HANDLE_VALUE || !buffer) {
            return AL_INVALID_PARAMETER;
        }

        BOOL unmapped = TRUE;
        BOOL closed = TRUE;

        if (buffer) {
            unmapped = UnmapViewOfFile(buffer);
        }

        if (mHandle && mHandle != INVALID_HANDLE_VALUE) {
            closed = CloseHandle(mHandle);
        }

        if (!unmapped) return AL_UNMAP_VIEW_FAILED;
        if (!closed) return AL_INTERNAL_UNKNOWN_SYSTEM_ERROR; 

        return ALGIS_OK;
    }

    // XSend and XRecieve
    inline ALGIS_STATUS __stdcall _xsend_(int transportType, LPCWSTR ipcName, const void *data, size_t dataSize) {
        if (!ipcName || !data || dataSize == 0) {
            return AL_INVALID_PARAMETER;
        }

        // named pipes
        if (transportType == XPIPE) {
            HANDLE hPipe = INVALID_HANDLE_VALUE;

            ALGIS_STATUS status = _openN_pipe_(ipcName, 5000, &hPipe);
            if (status != ALGIS_OK) {
                return status;
            }

            ipc_header_t header = {};
            std::memcpy(header.magic, IPC_MAGIC, sizeof(header.magic));
            header.PayloadSize = static_cast<std::uint32_t>(dataSize);

            std::vector<BYTE> packetBuffer(sizeof(ipc_header_t) + dataSize);
            std::memcpy(packetBuffer.data(), &header, sizeof(ipc_header_t));
            std::memcpy(packetBuffer.data() + sizeof(ipc_header_t), data, dataSize);

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

        // shared memory (no IPC_MAGIC yet)
        else if (transportType == XSMEM) {
            HANDLE hMap = NULL;
            void* pBuffer = NULL;

            ALGIS_STATUS status = _createSMem_(ipcName, static_cast<int>(dataSize), &hMap, &pBuffer);
            if (status != ALGIS_OK) {
                return status;
            }

            std::memcpy(pBuffer, data, dataSize);

            wchar_t eventName[256];
            swprintf_s(eventName, L"%s_EVENT", ipcName);

            HANDLE hEvent = OpenEventW(EVENT_MODIFY_STATE, FALSE, eventName); 
            if (hEvent) {
                SetEvent(hEvent);
                CloseHandle(hEvent);
            }

            _destroySMem_(hMap, pBuffer);
            return ALGIS_OK;
        }

        return AL_INVALID_PARAMETER;
    }
    inline ALGIS_STATUS __stdcall _xrecieve_(int transportType, LPCWSTR ipcName, void *buffer, size_t bufferSize, DWORD *bytesRead, DWORD timeoutMs) {
        if (!ipcName || !buffer || bufferSize == 0 || !bytesRead) {
            return AL_INVALID_PARAMETER;
        }

        *bytesRead = 0;

        // named pipes
        if (transportType == XPIPE) {
            HANDLE hPipe = INVALID_HANDLE_VALUE;

            size_t totalExpectedSize = sizeof(ipc_header_t) + bufferSize;
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

            if (!success || dwRead < sizeof(ipc_header_t)) {
                return AL_READ_FAILED;
            }

            ipc_header_t* header = reinterpret_cast<ipc_header_t*>(readBuffer.data());
            if (std::memcmp(header->magic, IPC_MAGIC, sizeof(header->magic)) != 0) {
                return AL_INTERNAL_UNKNOWN_SYSTEM_ERROR;
            }

            DWORD acutalPayloadSize = dwRead - sizeof(ipc_header_t);
            if (acutalPayloadSize > bufferSize) {
                acutalPayloadSize = static_cast<DWORD>(bufferSize);
            }

            std::memcpy(buffer, readBuffer.data() + sizeof(ipc_header_t), acutalPayloadSize);
            *bytesRead = acutalPayloadSize;
            return ALGIS_OK;
        }

        // shared memory
        else if (transportType == XSMEM) {
            wchar_t eventName[256];
            swprintf_s(eventName, L"%s_EVENT", ipcName);

            HANDLE hEvent = CreateEventW(NULL,FALSE, FALSE, eventName);
            if (!hEvent) return AL_INTERNAL_UNKNOWN_SYSTEM_ERROR;

            DWORD waitResult = WaitForSingleObject(hEvent, timeoutMs);
            if (waitResult == WAIT_TIMEOUT) {
                CloseHandle(hEvent);
                return AL_WAIT_TIMEOUT;
            }
            else if (waitResult != WAIT_OBJECT_0) {
                CloseHandle(hEvent);
                return AL_INTERNAL_UNKNOWN_SYSTEM_ERROR;
            }

            HANDLE hMap = NULL;
            void* pBuffer = NULL;

            ALGIS_STATUS status = _createSMem_(ipcName, static_cast<int>(bufferSize), &hMap, &pBuffer);
        
            if (status != ALGIS_OK) {
                CloseHandle(hEvent);
                return status;
            }

            std::memcpy(buffer, pBuffer, bufferSize);
            *bytesRead = static_cast<DWORD>(bufferSize);

            _destroySMem_(hMap, pBuffer);
            CloseHandle(hEvent);

            return ALGIS_OK;
        }

        return AL_INVALID_PARAMETER;
    }

	// internal dispatcher ctx
	struct _dispatch_ctx_ {
		std::wstring     pipeName;
		bool             running;
		std::thread      thread;
	};

	static _dispatch_ctx_ _ctx_ = {};

	// command handlers
	static ALGIS_STATUS _handle_ping_(LPCWSTR pipeName) {
		ipc_res_t res{};
		res.cmd_id = AL_PING;
		res.status = ALGIS_OK;
		res.size = 0;
		return detail::_xsend_(XPIPE, pipeName, &res, sizeof(ipc_res_t));
	}

	static ALGIS_STATUS _handle_data_(const ipc_cmd_t* cmd, const void* payload, LPCWSTR pipeName) {
		ipc_res_t res{};
		res.cmd_id = AL_DATA;
		res.status = ALGIS_OK;
		res.size   = 0;
		return detail::_xsend_(XPIPE, pipeName, &res, sizeof(ipc_res_t));
	}

	static ALGIS_STATUS _handle_execute_(const ipc_cmd_t* cmd, LPCWSTR pipeName) {
		ipc_res_t res{};
		res.cmd_id = AL_EXECUTE;
		res.status = ALGIS_OK;
		res.size   = 0;

		if (!cmd->addr) {
			res.status = AL_INVALID_PARAMETER;
			detail::_xsend_(XPIPE, pipeName, &res, sizeof(ipc_res_t));
			return AL_INVALID_PARAMETER;
		}
		// cast addr to a void function and call it
        reinterpret_cast<void(*)()>(cmd->addr)();
        return detail::_xsend_(XPIPE, pipeName, &res, sizeof(ipc_res_t));
	}

	// dispatcher loop
	static void _dispatcher_loop_(LPCWSTR pipeName) {
		while (_ctx_.running) {
			// read header + cmd in one shot
			size_t packetSize = sizeof(ipc_cmd_t) + 4096;
			std::vector<BYTE> buf(packetSize);
			DWORD bytesRead = 0;

			ALGIS_STATUS status = detail::_xrecieve_(XPIPE, pipeName, buf.data(), packetSize, &bytesRead, INFINITE);

			if (status != ALGIS_OK || bytesRead < sizeof(ipc_cmd_t)) continue;

			ipc_cmd_t* cmd = reinterpret_cast<ipc_cmd_t*>(buf.data());
			HOOK payload = buf.data() + sizeof(ipc_cmd_t);

			switch (static_cast<ALGIS_CMD>(cmd->cmd_id)) {
				case AL_PING:    _handle_ping_(pipeName);                break;
                case AL_DATA:    _handle_data_(cmd, payload, pipeName);  break;
                case AL_EXECUTE: _handle_execute_(cmd, pipeName);        break;
                default: break;
			}
		}
	}

#if defined(ALGIS_INTERNAL)
	struct _tp_xsend_ctx_ {
		int transportType;
		wchar_t ipcName[256];
		BYTE* data;
		size_t dataSize;
	};

	struct _tp_xrecieve_ctx_ {
		int transportType;
		wchar_t ipcName[256];
		DWORD timeoutMs;
		fnAlgisCallback callback;
	};

	static VOID CALLBACK _tp_xsend_cb_(PTP_CALLBACK_INSTANCE, PVOID ctx, PTP_WORK) {
		auto* c = static_cast<_tp_xsend_ctx_*>(ctx);
		detail::_xsend_(c->transportType, c->ipcName, c->data, c->dataSize);

		// heap was alloc by caller, free it
		HeapFree(GetProcessHeap(), 0, c->data);
		HeapFree(GetProcessHeap(), 0, c);
	}

	static VOID CALLBACK _tp_xrecieve_cb_(PTP_CALLBACK_INSTANCE, PVOID ctx, PTP_WORK) {
		auto* c = static_cast<_tp_xrecieve_ctx_*>(ctx);

		const size_t bufSize = 4096;
		BYTE* buf = static_cast<BYTE*>(HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, bufSize));
		if (!buf) { HeapFree(GetProcessHeap(), 0, c); return; }

		DWORD bytesRead = 0;
		ALGIS_STATUS status = detail::_xrecieve_(
			c->transportType, c->ipcName, buf, bufSize, &bytesRead, c->timeoutMs
		);

		if (status == ALGIS_OK && bytesRead > 0 && c->callback) {
			c->callback(buf, bytesRead);
		}

		HeapFree(GetProcessHeap(), 0, buf);
		HeapFree(GetProcessHeap(), 0, c);
	}

#endif //< @end ALGIS_INTERNAL
	} //< @end detail (internal)

	/*
	* Initialize this library, call this only ONCE ever, at the beginning of your program.
	*
	* IN - *processName -> Name of the process you want to target.
	* Usage: if (EH_Initialize("RobloxPlayerBeta.exe") != EH_OK) { return; }
	*/
	inline ALGIS_STATUS __stdcall hk_init(const char* processName) {
		return detail::_al_hk_init(processName);
	}
	/*
	* Uninitialize this library, call this only ONCE ever, at the end of your program.
	* Usage: (pretend we are at the end of the file) if (EH_Unitialize() != EH_OK) { return; }
	*/
	inline ALGIS_STATUS __stdcall hk_uninit(VOID) {
		return detail::_al_hk_uninit();
	}
	/*
	* Maps your dll into memory via your target PID
	* IN ->  *DLLPath -> THe absoulte path of your DLL.
	* OUT -> *oEntryPoint -> The address of your entry point. (NEEDED)
	*/
	inline ALGIS_STATUS __stdcall map(const wchar_t* dllPath, uintptr_t* oEntryPoint) {
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
	inline ALGIS_STATUS __stdcall create_hook(void* jmpInstruction, void* newTarget, HOOK* oHook) {
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
	inline ALGIS_STATUS __stdcall destroy_hook(HOOK hook) {
		return detail::_a_destroy_hook(hook);
	}

    // IPC section

    /*
    * CreateNPipe
    * Used to create a standard bi-directonal named pipe. (!!FOR THE SERVER!!)
    *
    * IN -> pipeName -> Name of your pipe, MUST START WITH "\\\\.\\pipe\\[your_name]"
    * IN -> bufferSize -> Output buffer size and input buffer size
    * OUT -> *oPipeHandle -> The pipe handle, will be used to destroy your pipe
    */
    inline ALGIS_STATUS __stdcall createNPipe(LPCWSTR pipeName, DWORD bufferSize, HANDLE* oPipeHandle) {
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
    inline ALGIS_STATUS __stdcall openNPipe(LPCWSTR pipeName, DWORD timeoutMs, HANDLE* oPipeHandle) {
        return detail::_openN_pipe_(pipeName, timeoutMs, oPipeHandle);
    }

    /*
    * DestroyNPipe
    * Used to destroy a named pipe created with CreateNPipe,
    *
    * IN -> *pipeHandle -> The handle to your pipe.
    */
    inline ALGIS_STATUS __stdcall destroyNPipe(HANDLE* hPipe) {
        return detail::_destroyN_pipe_(hPipe);
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
    inline ALGIS_STATUS __stdcall xsend(int transportType, LPCWSTR ipcName, const void *data, size_t dataSize, bool seperateThread = false) {
        if (!ipcName || !data || dataSize == 0) return AL_INVALID_PARAMETER;

        if (seperateThread) {
            std::vector<BYTE> dataCopy(static_cast<const BYTE*>(data), static_cast<const BYTE*>(data) + dataSize);
            std::wstring nameCopy(ipcName);

            std::thread([transportType, nameCopy, dataCopy]() {
                detail::_xsend_(transportType, nameCopy.c_str(), dataCopy.data(), dataCopy.size());
                }).detach();

            return ALGIS_OK;
        } else {
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
    inline ALGIS_STATUS __stdcall xrecieve(int transportType, LPCWSTR ipcName, void* oBuffer, size_t bufferSize, DWORD* oBytesRead, DWORD timeoutMs, fnAlgisCallback callback = nullptr, bool seperateThread = false) {
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
        } else {
            return detail::_xrecieve_(transportType, ipcName, oBuffer, bufferSize, oBytesRead, timeoutMs);
        }
    }

	// dispatcher loop for IPC
	inline ALGIS_STATUS dispatcher_start(LPCWSTR pipeName) {
		if (detail::_ctx_.running) return AL_INVALID_PARAMETER;
		detail::_ctx_.pipeName = pipeName;
		detail::_ctx_.running = true;
		detail::_ctx_.thread = std::thread(detail::_dispatcher_loop_, pipeName);
		return ALGIS_OK;
	}

	inline ALGIS_STATUS dispatcher_stop() {
		if (!detail::_ctx_.running) return AL_INVALID_PARAMETER;
		detail::_ctx_.running = false;
		if (detail::_ctx_.thread.joinable()) {
			detail::_ctx_.thread.join();
		}
		return ALGIS_OK;
	}

#if defined(ALGIS_EXTERNAL)
	inline std::unique_ptr<detail::memory_t>& Memory() {
		static std::unique_ptr<detail::memory_t> instance = std::make_unique<detail::memory_t>();
		return instance;
	}
#endif
	
#if defined(ALGIS_INTERNAL) 
	inline std::unique_ptr<detail::internal_memory_t>& memory() {
		static std::unique_ptr<detail::internal_memory_t> instance = std::make_unique<internal_memory_t>();
		return instance;
	}

	inline ALGIS_STATUS __stdcall xsend_tp(int transportType, LPCWSTR ipcName, const void* data, size_t dataSize) {
    if (!ipcName || !data || dataSize == 0) return AL_INVALID_PARAMETER;

    auto* ctx = static_cast<detail::_tp_xsend_ctx_*>(
        HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(detail::_tp_xsend_ctx_))
    );
    if (!ctx) return AL_INTERNAL_UNKNOWN_SYSTEM_ERROR;

    BYTE* dataCopy = static_cast<BYTE*>(HeapAlloc(GetProcessHeap(), 0, dataSize));
    if (!dataCopy) { HeapFree(GetProcessHeap(), 0, ctx); return AL_INTERNAL_UNKNOWN_SYSTEM_ERROR; }

    std::memcpy(dataCopy, data, dataSize);
    ctx->transportType = transportType;
    ctx->dataSize      = dataSize;
    ctx->data          = dataCopy;
    lstrcpynW(ctx->ipcName, ipcName, 256);

    PTP_WORK work = CreateThreadpoolWork(detail::_tp_xsend_cb_, ctx, nullptr);
    if (!work) {
        HeapFree(GetProcessHeap(), 0, dataCopy);
        HeapFree(GetProcessHeap(), 0, ctx);
        return AL_INTERNAL_UNKNOWN_SYSTEM_ERROR;
    }

    SubmitThreadpoolWork(work);
    CloseThreadpoolWork(work);
    return ALGIS_OK;
	}

	inline ALGIS_STATUS __stdcall xrecieve_tp(int transportType, LPCWSTR ipcName, DWORD timeoutMs, fnAlgisCallback callback) {
    if (!ipcName || !callback) return AL_INVALID_PARAMETER;

    auto* ctx = static_cast<detail::_tp_xrecieve_ctx_*>(
        HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(detail::_tp_xrecieve_ctx_))
    );
    if (!ctx) return AL_INTERNAL_UNKNOWN_SYSTEM_ERROR;

    ctx->transportType = transportType;
    ctx->timeoutMs     = timeoutMs;
    ctx->callback      = callback;
    lstrcpynW(ctx->ipcName, ipcName, 256);

    PTP_WORK work = CreateThreadpoolWork(detail::_tp_xrecieve_cb_, ctx, nullptr);
    if (!work) {
        HeapFree(GetProcessHeap(), 0, ctx);
        return AL_INTERNAL_UNKNOWN_SYSTEM_ERROR;
    }

    SubmitThreadpoolWork(work);
    CloseThreadpoolWork(work);
    return ALGIS_OK;
	}
#endif

} //< @end algis

#endif // ALGIS_H
