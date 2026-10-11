#include <windows.h>
#include "../include/SDK/sdk.hpp"



extern "C" int _fltused = 0;

void* operator new(size_t size) { return HeapAlloc(GetProcessHeap(), 0, size); }
void* operator new[](size_t size) { return HeapAlloc(GetProcessHeap(), 0, size); }
void operator delete(void* ptr) noexcept { HeapFree(GetProcessHeap(), 0, ptr); }
void operator delete[](void* ptr) noexcept { HeapFree(GetProcessHeap(), 0, ptr); }

#pragma function(memset)
#pragma function(memcpy)

extern "C" {
    unsigned __int64 __security_cookie = 0;
    void __cdecl __security_check_cookie(unsigned __int64 cookie) { (void)cookie; }

    void __cdecl __std_terminate() { TerminateProcess(GetCurrentProcess(), 0); }

    int __cdecl __CxxFrameHandler4(void*, void*, void*, void*) { return 0; }

    EXCEPTION_DISPOSITION __cdecl __C_specific_handler(
        struct _EXCEPTION_RECORD* rec,
        void* frame,
        struct _CONTEXT* ctx,
        struct _DISPATCHER_CONTEXT* disp)
    {
        return ExceptionContinueSearch;
    }

    // range check failure — just terminate
    void __cdecl __report_rangecheckfailure() {
        TerminateProcess(GetCurrentProcess(), 0);
    }

    // GS handler check stub
    void __cdecl __GSHandlerCheck() {}

    // memset/memcpy intrinsics
    void* __cdecl memset(void* dst, int val, size_t size) {
        unsigned char* p = (unsigned char*)dst;
        while (size--) *p++ = (unsigned char)val;
        return dst;
    }

    void* __cdecl memcpy(void* dst, const void* src, size_t size) {
        unsigned char* d = (unsigned char*)dst;
        const unsigned char* s = (const unsigned char*)src;
        while (size--) *d++ = *s++;
        return dst;
    }
}

extern "C" __declspec(dllexport) void ThreadpoolCallback()
{

    static bool already_ran = false;
    if (already_ran) return;
    already_ran = true;

    RBX::Log::Init();

    uintptr_t dm = RBX::Fetch.GetDM();
    uintptr_t ts = RBX::Fetch.GetTS();

    RBX::Log::Print("DataModel: [0x%llx]\n", dm);
    RBX::Log::Print("TaskScheduler: [0x%llx]\n", ts);

    if (dm) {
        RBX::Instance workspace(dm);
        RBX::Instance ws = workspace.FindFirstChild("Workspace");
        RBX::Log::Print("Workspace: [0x%llx] : [%s]\n", ws.GetAddress(), ws.GetName());
    }

    RBX::Log::Close();
}
