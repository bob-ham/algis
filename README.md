## ALGIS.H ##

*A huge header file, filled with a bunch of cheating tools. A skids dream.*

**CREATED BY FREAKSTER, AKA <ALGIS> on TikTok and Instangram**

**SOME MACRO DEFS**
- `__attr_pack__`, defines to `_Pragma("pack(push, 1)")`
- `__attr_pop__`, defines to `_Pragma("pack(pop)")`
- `HOOK`, used a lot cause im lazy, defines to `void*`
- `fnAlgisCallback`, defines to `void* data, size_t size`

- Check License for more details. 
- Most of the documentation for this is inside of algis.h
- However, an overview

*DLL INJECTION*
- manual mapper + IAT hooker, detected on most games but gets the job done
- has two main classes, one struct `Process` class and `mapper` class, `Hook` struct

*SYSCALL RESOLVER*
- instead of using NTAPI, which used to be my old code, i decided to switch to syscalls, which works quite well.
- if there's sudden bugs, call `syscall::Init()` as i might have forgotten to add that in the section of code you are using.

*EXTERNAL MEMORY MANIPULATION*
- uses `SYSCALL RESOLVER` to call `OpenProcess`, which if failed, uses a custom handle hijacker.
- has an external memory class, `memory_t`, which is converted to a smart pointer to be used as such, `algis::Memory().Read` (example)

*IPC PROTOCOL*
- this will be the most information dense, as it has a lot of moving parts
- uses a few macros, `XPIPE`, `XSMEM`, `IPC_THREAD_CREATE`, `IPC_MAGIC`
- `XPIPE` is just 1; named pipe
- `XSMEM` 2; shared memory
- `IPC_THREAD_CREATE` true; optional boolean to create a thread (DO NOT USE INTERNALLY UNLESS YOU CAN MAKE THREADS/HAVE CRT)
- `IPC_MAGIC` "algis_ipc"; magic header to confirm
- now the structs, im just going to explain briefly, `typedef struct _ipc_header` -> `uint16_t transportType; uint32_t payloadsize` (transport -> XPIPE, XSMEM)
- `typedef struct _ipc_cmd` -> `uint16_t cmd_id, flags; uint64_t addr; uint32_t size, reserved`
-  `typedef enum _ALGIS_CMD : uint16_t` -> `AL_PING = 0x01, AL_DATA = 0x02, AL_EXECUTE = 0x03` if you want more, write your own defs idk
-  now the dispatcher manager, a few structs now
-  `struct _dispatch_ctx_` -> `LPCWSTR pipeName; bool running; std::thread thread;`
-  now internally its a bit different
-  just the same stuff here except `IPC_THREAD_CREATE` would be equal to putting it off to the threadpool unless you just wanna create it in another thread

*ENCRYPTION*
- encryption, so far there is string literal obfuscation and pointer obfuscation, based in a few classes/structs/namespaces
- the important stuff is mainly `namespace _int_obf_` which is an internal obfuscation namespace for string literals
- for SL (string literal), the struct is `typedef struct _obf_res` -> `char* data; size_t size; uint64_t key; int level;` further elaboration is in code
- `class encrypt final` (should be sl_encrypt but im lazy) -> has a simple decrypt function and wipe function
- `typedef struct _obf_ptr` -> `uint32_t lo, noise0, h1, noise1;`
- `class ptr_encrypt final` -> again, another simple decrypt and wipe function
- `SENCRYPT_L1(str)` -> encrypts your string simply, doesnt do much
- `SENCRYPT_L2(str)` -> encrypts your string, with a bit more obfuscation
- `SENCRYPT_L3(str)` -> highest level of string encryption i made
- `PENCRYPT(enc)` -> only one level of pointer encryption
- `PDECRYPT(enc)` -> self explaintory

*FLogger*
- `class FLogger` -> a simple logger class, raw win32 so it doesnt need CRT exactly (might have to do some tweaking though)
- `LOG_INIT(filename)` -> where your log file will be (for example C:\\Users\\Public\\algis.log)
- `ILOG(fmt, ...)` -> logs your information with an "INFO" tag and timestamp
- `WLOG(fmt, ...)` -> logs your information with an "WARN" tag and timestamp
- `ELOG(fmt, ...)` -> logs your information with an "ERROR" tag and timestamp

## WHAT I MIGHT ADD ##
- network helper, fetching JSON but a JSON parser is terrible; might just make it parse .hpp files instead
- more internal helpers, vmt hooker perhaps?
