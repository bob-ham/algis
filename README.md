## ALGIS 
*a cheat library, filled with a collection of tools i made over the years.*

**CREATED BY FREASKTER, AKA \<algis.h> on TikTok and Instagram.** 

**Discord is `ownerofsomethingbruh`**

*check [LICENSE](https://github.com/bob-ham/algis/blob/master/LICENSE) for license details*

**HOW TO USE**
- basic CMake setup:
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

