// algis.h
// clang-format off
#ifndef ALGIS_H
#define ALGIS_H
/* 
MIT License

Copyright (c) 2026 bob-ham

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/


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

// ALGIS_LEAN     < cuts out the networking part of the lib
// ALGIS_EXTERNAL < user defined; are you internal, or external?
// ALGIS_INTERNAL < user defined; are you internal, or external?
// ALGIS_LUA      < lua impl; just something i think is cool
// ALGIS_FULL     < pulls in missing headers, such as memory_t.hpp, dispatch.hpp and ipc_log.hpp 


// !! SEPERATED CODE !!

// system headers
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <ntstatus.h>
#include <winternl.h>
#include <intrin.h>
#include <tlhelp32.h>
#if !defined(ALGIS_LEAN)
// #include <wininet.h>
// #include <regex>
// #pragma comment(lib, "wininet.lib")
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

// constants
#ifndef XPIPE
#define XPIPE 1 //< used in named pipes
#endif

#ifndef XSMEM
#define XSMEM 2 //< used in shared memory
#endif

#ifndef DESTROY_HOOK_ALL
#define DESTROY_HOOK_ALL nullptr //< used to destroy every hook
#endif

// change; removed IPC_THREAD_CREATE, as xsend threaded was disreguarding the return and just return ALGIS_OK
// if user wants to do multi-threaded work, they can just do it manually


// change; removed IPC_MAGIC macro as its an internal value that doesnt need to be exposed to the public
// also removed typedef HOOK and other NTAPI macros/typedefs
// HOOK replaced by void* and void** in function defs
// removed fnAlgisCallback as IPC_THREAD_CREATE was also removed

#ifndef ALGIS_VERSION
#define ALGIS_VERSION "V1.1.2" // something to help track when i update
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

// logger, which im just going to define here as its easier
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
#define LOG_INIT(file_path) FLogger::GetLogInstance().LogInit(filename)
#define ILOG(fmt, ...)     FLogger::GetLogInstance().Log("INFO", fmt, ##__VA_ARGS__)
#define WLOG(fmt, ...)     FLogger::GetLogInstance().Log("WARN", fmt, ##__VA_ARGS__)
#define ELOG(fmt, ...)     FLogger::GetLogInstance().Log("ERROR", fmt, ##__VA_ARGS__)

// change; im not sure why i used typedef here

enum ALGIS_STATUS : std::uint32_t {
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
};

// lil helper
#define ALGIS_API ALGIS_STATUS __stdcall

// change; removed __attr_pack__ and __attr_pop__ (i see what that one guy was talking about when he said "neuroslop")
// im not sure why i used typedef here

struct IpcHeader {
    char magic[10]; // stores "algis_ipc"
    std::uint16_t transport;
    std::uint32_t payload_size;
};

struct IpcCmd {
    std::uint16_t cmd_id;
    std::uint16_t flags;
    std::uint64_t addr;
    std::uint32_t size;
    std::uint32_t reserved;
};

struct IpcReserve {
    std::uint16_t cmd_id;     // mirrors the command that triggered this
    ALGIS_STATUS  status;     // result
    std::uint32_t size;       // size of data[] if any 
	BYTE          data[512];  // response payload
};

struct IpcLog {
	std::uint8_t  level;    // info, warn, error
	std::uint32_t size;     // length of message
	char          msg[512]; // the log string
};

enum class AlgisCmd : std::uint16_t {
    AL_PING    = 0x01, // alive check, no payload (external->internal)
	AL_DATA    = 0x02, // send data blob (external->internal)
	AL_EXECUTE = 0x03, // stub, for the user to decide (external->internal)
	AL_LOG     = 0x04, // logs to the ipc (internal->external)
};



// function defs

namespace algis {
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
	#if defined(ALGIS_EXTERNAL)
	    /*
		 * hook_init
		 * Initialize this library, call this only ONCE ever, at the beginning of your program.
		 *
		 * @IN -> *process_name : Name of the process you want to target.
		 *
		 * Usage: if (EH_Initialize("RobloxPlayerBeta.exe") != EH_OK) { return; }
		 */
	    ALGIS_API hook_init(const char* process_name);

		/*
		 * hook_uninit
		 * Uninitialize this library, call this only ONCE ever, at the end of your program.
		 *
		 * Usage: (pretend we are at the end of the file) if (EH_Unitialize() != EH_OK) { return; }
		 */
	    ALGIS_API hook_uninit(VOID);

	    /*
		 * map
		 * Maps your dll into memory via your target PID.
		 *
		 * @IN  -> *dll_path      : the absoulte path of your DLL.
		 * @OUT -> *o_entrypoint  : the address of your entry point. (NEEDED)
		 */
	    ALGIS_API map(const wchar_t* dll_path, std::uintptr_t* o_entrypoint);

		/*
		 * create_hook
		 * Creates an external hook via IAT hooking.
		 *
		 * @IN  -> *jmp_instruction : the function you want to hook on.
		 * @IN  -> *new_target      : your exported function in your dll (edit mmap.cpp for that).
		 * @OUT -> *o_hook          : stores the hook pointer, to destroy later with destroy_hook.
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
	    ALGIS_API create_hook(void* jmp_instruction, void* new_target, void** o_hook);

		/*
		 * destroy_hook
		 * Call this function to clean up a specific hook.
		 * If you want to destroy every hook, use DESTROY_HOOK_ALL.
		 *
		 * @IN -> hook : cleans only one specific hook.
		 *
		 * Usage: EH_STATUS status = EH_CleanHook(qpcHook);
		 * OR
		 * EH_STATUS status = EH_CleanHook(EH_DESTROY);
		 */
	    ALGIS_API destroy_hook(void* hook);

		/*
		 * driver()
		 * 
		 * external process manip, a smart pointer that has these commands
		 * @Read -> reads obviosuly
		 * @Write
		 * @GetModuleAddress
		 * @HijackHandle
		 * so and so forth!
		 * !! LOCATED IN memory_t.hpp !!
		 */
		
	#endif
	// global code

	/*
	 * create_pipe
	 * Used to create a standard bi-directonal named pipe. (!!FOR THE SERVER!!)
	 *
	 * @IN  -> pipe_name   : name of your pipe, MUST START WITH "\\\\.\\pipe\\[your_name]"
	 * @IN  -> buffer_size : output buffer size and input buffer size
	 * @OUT -> *o_handle   : the pipe handle, will be used to destroy your pipe
	 */
	ALGIS_API create_pipe(LPCWSTR pipe_name, DWORD buffer_size, HANDLE* o_handle);

	/*
	 * open_pipe
	 * Used to connect/open a existing named pipe. (!!FOR THE CLIENT!!)
	 *
	 * @IN  -> pipe_name : name of your pipe
	 * @IN  -> ms        : timeout miliseconds
	 * @OUT -> *o_handle : the pipe handle, used to send data to the server.
	 */
	ALGIS_API open_pipe(LPCWSTR pipe_name, DWORD ms, HANDLE* o_handle);

	/*
	 * destroy_pipe
	 * Used to destroy a named pipe created with create_pipe.
	 *
	 * @IN -> *handle : the handle to your pipe.
	 */
	ALGIS_API destroy_pipe(HANDLE* handle);

	/*
	 * create_smem
	 * used to create shared memory (kind of bare right now)
	 * 
	 * @IN  -> mem_name    : ipc name
	 * @IN  -> buffer_size : size of your buffer
	 * @OUT -> *o_handle   : output handle you get to manage your smem
	 * @OUT -> *o_buffer   : output buffer you get to manage you smem
	 * 
	 * might change it so HANDLE has a RAII wrapper around it, but after
	 */
	ALGIS_API create_smem(LPCWSTR mem_name, int buffer_size, HANDLE *o_handle, void** o_buffer);

	/*
	 * open_smem
	 * used to open smem, client
	 * 
	 * @IN  -> mem_name    : ipc name
	 * @IN  -> buffer_size : size of your buffer
	 * @OUT -> *o_handle   : output handle you get to manage
	 * @OUT -> *o_buffer   : output handle you get to manage
	 */
	ALGIS_API open_smem(LPCWSTR mem_name, int buffer_size, HANDLE *o_handle, void** o_buffer);

	/*
	 * destroy_smem
	 * used to destroy shared memory
	 * 
	 * @IN -> o_handle : input handle to destroy shared memory
	 * @IN -> *buffer  : input buffer to destroy shared memory
	 */
	ALGIS_API destroy_smem(HANDLE o_handle, void* buffer);

	/*
	 * xsend
	 * sends whatever you want, to whatever IPC
	 * 
	 * @IN  -> transport_type  : you will use either one of two values, XPIPE (1) for named pipes and XSMEM (2) for shared memory
	 * @IN  -> ipc_name        : the exact name of what you put for your IPC
	 * @IN  -> *data           : the data you wish to send
	 * @IN  -> data_size       : the size of the data your entering
	 */
	ALGIS_API xsend(int transport_type, LPCWSTR ipc_name, const void *data, size_t data_size);

	/*
	 * xrecieve
	 * recieves data from IPC
	 * 
	 * @IN  -> transport_type  : XPIPE (1) or XSMEM (2)
	 * @IN  -> ipc_name        : the exact name of your IPC
	 * @OUT -> *o_buffer       : buffer to store incoming data
	 * @IN  -> buffer_size     : capacity of *o_buffer
	 * @OUT -> bytes_read      : number of bytes acutally read/recieved
	 * @IN  -> ms              : time in miliseconds to wait for data before timing out (INFINITE can be used)
	 */
	ALGIS_API xrecieve(int transport_type, LPCWSTR ipc_name, void *o_buffer, size_t buffer_size, DWORD *o_bytes_read, DWORD ms);


	// internal IPC
#if defined(ALGIS_INTERNAL)
	/*
	 * ipc_log
	 * takes data from an IPC and logs it to a file (reason why its internal is because why would you use this externaly???)
	 * confused; use xrecieve to receive the data from this, then cast it to IpcLog via reinterpret_cast<IpcLog*>(buf + sizeof(IpcCmd)) (IpcCmd because AL_LOG is in IpcCmd)
	 * 
	 * @IN -> ipc_name : name of your ipc
	 * @IN -> level    : info, warning, error, etc etc
	 * @IN -> msg      : what msg you want to put in
	 */
	ALGIS_API ipc_log(LPCWSTR ipc_name, std::uint8_t level, const char* msg);

	/*
	* dispatch
	* IPC dispatcher class, runs in its own thread, handles IpcCmd
	* include internal/dispatch.hpp to use the full class, make sure to define
	* ALGIS_INTERNAL
	 */
#endif

	// NETWORKING
#if !defined(ALGIS_LEAN)
// nothing, messed up here a LOT so ill deal with it later
#endif 
} // namespace algis


// if algis full
#if defined(ALGIS_FULL)
  #include "global/memory_t.hpp"
  #if defined(ALGIS_INTERNAL)
    #include "internal/dispatch.hpp"
    #include "internal/ipc_log.hpp"
  #endif
#endif


#endif // ALGIS_H