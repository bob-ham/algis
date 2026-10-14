## ALGIS 
*a cheat library, filled with a collection of tools i made over the years.*

**CREATED BY FREASKTER, AKA \<algis.h> on TikTok and Instagram.** 

**Discord is `ownerofsomethingbruh`**

**WHAT IT IS**

*algis is a big cheating lib i made over the years. i decided to make it as i wanted to exit the roblox cheating scene for many personal reason
i have exited the scene but i want to leave behind the stuff i used. i want to contribute to the scene without directly writing cheats/execs.
thats the entire purpose of algis, a lazy way for me to contribute without having to do anything*

## GETTING STARTED
- i recommend to use a CMake, as it makes compilation easier
- was also made in c++ if that wasnt obvious
- basic cmake:
```cmake
cmake_minimum_required(VERSION 3.20)
project(test LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

# define before including
add_definitions(-DALGIS_EXTERNAL)

add_executable(test main.cpp)

target_include_directories(test PRIVATE algis/algis.h)
```
- then in code, define your macro and include:
```cpp
#define ALGIS_EXTERNAL // or ALGIS_INTERNAL if dll
#include "../includes/algis.h"
```

## USEFUL KNOWLEDGE
- every function in the algis:: namespace uses the `ALGIS_API` macro
- `ALGIS_API` expands to `ALGIS_STATUS __stdcall`
- `ALGIS_STATUS` is a big enum that expands to this
```cpp
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
```
- this is the main enum, every function in algis:: uses this
- there is also a `al_to_string` function that takes an `ALGIS_STATUS` status and converts it to a string
- there is two main parts to this, the memory/injection and IPC. i will add more, but those are the main parts. keep that in mind

## USAGE
- ill go from least important to most important
- `FLogger` class, just a simple logger class i made
- you use the macros `LOG_INIT` and put in a file_path parameter
- then there is 3 macros, `ILOG`, `WLOG`, `ELOG` which all take a message
- example:
```cpp
// defined all of your previous stuff up there..
int main() {
  LOG_INIT("C:\\Users\\public\\log.log");

  std::string msg = "hi";
  ILOG("coolio");
  WLOG("message: %s", msg.c_str());
}
```
**MEMORY**
- `memory_t` and `internal_memory_t`
- `memory_t` expands to this def, `class memory_t final : public Process`
- the `Process` class pulls from my syscall resolver, and `memory_t` is just a name steal from
- a man called stackz, his channel is [HERE](https://www.youtube.com/@stackzeu)
- `memory_t` includes simple stuff such as read/write commands, a handle hijacker, module address getter,
- such and such is from `Process` class mainly
- its basically everything needed for the base of an external
- `internal_memory_t` is the same as `memory_t` but internal, obviously you dont need a
- handle hijacker for it, so its just read/write commands + module address getter
- both `memory_t` and `internal_memory_t` and turned into smart pointers

**DLL INJECTOR**
- DLL injector now, seperated into 4 parts
- `hook.hpp`, `mmap.cpp/hpp`, `process.cpp/hpp` are the main parts, there is one more (`syscall.hpp`) but i wont talk about it here
- everything starts with `process.cpp/hpp`
- as you can guess, it includes the `Process` class
- the `Process` class looks like this:
```cpp
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

  NTSTATUS read_memr(PVOID base, PVOID buf, SIZE_T size, PSIZE_T read);
  NTSTATUS write_memr(PVOID base, PVOID buf, SIZE_T siz, PSIZE_T written);
  NTSTATUS change_protection(PVOID base, SIZE_T size, ULONG prot, PULONG oldProt);
  PVOID allocate_mem(SIZE_T size, ULONG type, ULONG prot);

  void suspend_all_threads(DWORD pid, bool suspend);
};
```
- the main important functions to look at here:
- `HANDLE hijack_handle(DWORD target_pid)`, and `suspend_all_threads`
- `hijack_handle` is too much code to paste here, so its better to look at the repo itself but as it suggests
- it hijacks a handle via syscalls instead of using `OpenProcess`, now my target was roblox which i assume is for
- you guys too, but `OpenProcess` never worked for me so i had to use this. not detected as far as i know
- `suspend_all_threads` is essential for nothing but the IAT hooker, and i talked about it because it fixes
- a lot of bugs i had with it.

- now `mmap.cpp/hpp`
- the manual mapper is a simple class called `mapper`, and it looks like this
```cpp
class mapper {
private:
  Process& process;
  bool fix_relocations(PVOID localBase, uintptr_t remoteBase);
  bool resolve_imports(PVOID localBase);
  bool register_seh(PVOID localBase, uintptr_t remoteBase, SIZE_T imageSize);

public:
  mapper(Process& process);

  struct map_result {
    std::uintptr_t remote_base;
    std::uintptr_t entry_point;
    bool success;
  };

  map_result map(const std::wstring& dll_path);
};
```
- the important thing to take away is that it takes the `Process` class as a function and makes a
- `map_result` struct, important for the IAT hooker
- the three main functions are `fix_relocations`, `resolve_imports` and `register_seh`
- `fix_relocations` fixes DLL relocations, pretty self explantory
- `resolve_imports` resolves the IAT table of the DLL
- `register_seh` registers Structured Exception Handling
- the main function, `map` maps your dll into process memory, there is a few takeaways
- there is no CRT (C Runtime Library) which means no std::string, std::vector, such and such (i am working on a fix for this)
- also it only really works on small DLL's, but i am trying to fix these

- finally, `hook.hpp`, which defines two structs
```cpp
struct Hook {
  void** target_slot;
  void* orig;
  // hook
  bool hook(Process& proc, void* jmpInstruction, void* newTarget) {
    /*
     * Calculate the address of the pointer slot.
     * jmp [rip + offset] - offset is at byte 3. Pointer is at instruction + 7 + offset.
     */
    int32_t offset = 0;
    SIZE_T read;
    proc.read_memr((PVOID)((uintptr_t)jmpInstruction + 3), &offset, sizeof(int32_t), &read);
    target_slot = (void**)((uintptr_t)jmpInstruction + 7 + offset);

    /* Read original memory. */
    proc.read_memr(target_slot, &orig, sizeof(void*), &read);

    /* Write the new target. */
    ULONG oldProt;
    proc.change_protection(target_slot, sizeof(void*), PAGE_EXECUTE_READWRITE, &oldProt);
    SIZE_T written;
    proc.write_memr(target_slot, &newTarget, sizeof(void*), &written);
    proc.change_protection(target_slot, sizeof(void*), oldProt, &oldProt);

    /* We are hooked! */
    return true;
  }
  /* Unhook. */
  bool unhook(Process& proc) {
    ULONG oldProt;
    proc.change_protection(target_slot, sizeof(void*), PAGE_EXECUTE_READWRITE, &oldProt);
    SIZE_T written;
    proc.write_memr(target_slot, &orig, sizeof(void*), &written);
    proc.change_protection(target_slot, sizeof(void*), oldProt, &oldProt);
    /* We are unhooked! */
    return true;
  }
};
```
- the main functions here are `hook` and `unhook`
- `hook` installs the IAT hook via the `Process` class
- `unhook` removes said hook, pretty self explantory honesntly
- the second struct is just a hook lib, it is here
```cpp
struct HookLib {
  Process* process = nullptr;
  std::map<void*, Hook*> registry;
  mapper::map_result last_map_result = {0, 0, false};
};

inline HookLib& GetHookLib() {
  static HookLib lib;
  return lib;
}
```
- i dont think any explanation is needed, pretty self explantory

**HOOK EXAMPLE**
- now the final part, `algis.cpp`
- ill go through the 5 functions that is this
- `ALGIS_API algis::init(const char* process_name)`; takes your target (lets use RobloxPlayerBeta.exe) and gets a handle to it/inits the `Process` class
- `ALGIS_API algis::uninit(VOID)`; destroys the process class basically
- `AGLIS_API algis::map(const wchar_t* dll_path, std::uintptr_t* o_entry)`; takes your dll math, manually maps it and returns the entry point
- `AGLIS_API algis::create_hook(void* jmp_instruction, void* new_target void** o_hook)`; takes your desired target `jmp_instruction`, puts in new target (entry point from DLL) and outputs the address to the hook `o_hook`
- `ALGIS_API algis::destroy_hook(void* hook)` gets said outputted hook and removes it, its called like `algis::destroy_hook` then `algis::uninit`
- an example file will be [here](https://github.com/bob-ham/algis/blob/master/repos/algis/examples/example_hook.cpp)

**IPC**
- the IPC part of algis, was very fun to make
- there is 5 main structs we need to talk about, and 2 public macros
- the macros, `XPIPE` and `XSMEM`, both equal 1 and 2 respectively. XPIPE is used for `algis::xsend` and `algis::xrecieve`, same with XSMEM (dont worry about this right now, ill explain it later)
- the 5 main structs, `IpcHeader`, `IpcCmd`, `IpcReserve`, `IpcLog` and a enum `AlgisCmd`
- each struct holds these
```cpp
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
```
- most of this is common, now most of these are specific to the dispatcher, but ill talk about that later. first lets talk about the common ones
- `struct IpcHeader`, holds three values. `char magic[10]` which stores the magic header "algis_ipc", `std::uint16_t transport` which is the transport type `XPIPE` or `XSMEM` and `std::uint32_t payload_size` which is just how big the data your sending is.
- im going to go into the two main functions here, `algis::xsend` and `algis::xrecieve`, the other functions are easy to understand. they both look like this
```cpp
ALGIS_API xsend(int transport_type, LPCWSTR ipc_name, const void *data, size_t data_size);
ALGIS_API xrecieve(int transport_type, LPCWSTR ipc_name, void *o_buffer, size_t buffer_size, DWORD *o_bytes_read, DWORD ms);
```
- `xsend` takes 4 args. transport_type `XPIPE` or `XSMEM`, name of IPC, the data, and size of data. `xsend` is for the client
- `xrecieve` takes 6 args. transport_type, name of ipc, output buffer, buffer_size, output_bytes_read and timeout milliseconds. `xrecieve` is for the server
- an example file will be listed [here](https://github.com/bob-ham/algis/blob/master/repos/algis/examples/example_ipc.cpp)

## WHAT I AM PLANNING TO ADD
- i want to add a lua/ branch to this
- more QOL, more code clean up
- i am also going to soon add videos to show proof that this works

## LICENSE
*check [LICENSE](https://github.com/bob-ham/algis/blob/master/LICENSE) for license details*
