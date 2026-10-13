ALGIS ##
*a cheat library, filled with a collection of tools i made over the years. the dream of a skid*
**CREATED BY FREASKTER, AKA <algis.h> on TikTok and Instagram. Discord is `ownerofsomethingbruh`**
*check [https://github.com/bob-ham/algis/blob/master/LICENSE](LICESNSE) for license details*

**STRUCTURE**
- since this is no longer algis v1.0 where it was a single header file (now deprecated, i made it as a challenge and i maded making it myself)
- everything is split in this file structure
- algis.h <- master header file, include this. most documentation is here
- seperated/
-  impl/
-   algis.cpp <- function defs
-  globals/
-   ipc.hpp      <- IPC impl
-   memory_t     <- external and internal memory tools
-   network.hpp  <- offset fetcher *(ONLY FETCHES .HPP FILES!)*
-   syscalls.hpp <- syscall resolver
- external/
-  hooker/
-   process.hpp/cpp <- process class, handle hijacking and raw memory reading
-   hook.hpp        <- IAT hooker
-   mmap.hpp/cpp    <- manual mapper
- internal/
-  dispatch.hpp/cpp <- IPC dispatcher
-  ipc_log.hpp      <- internal logging over IPC

**MACROS**
- define these <i>BEFORE<i> including algis.h
