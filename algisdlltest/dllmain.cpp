#include <windows.h>
#define ALGIS_INTERNAL
#include "../algis/vendor/algis.h"

extern "C" {
    extern BOOL WINAPI _CRT_INIT(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved);
}

HINSTANCE g_ModuleBase = nullptr;


DWORD WINAPI test(LPVOID lpParam) {

    OutputDebugStringA("background thread created\n");

    while (true) {
        Sleep(100);
    }
    return 0;
}

extern "C" __declspec(dllexport) void ThreadpoolCallback() {
    static volatile LONG s_Init = 0;

    if (InterlockedExchange(&s_Init, 1) == 0) {
        uintptr_t current_address = (uintptr_t)&ThreadpoolCallback;
        uintptr_t calculated_base = current_address & ~0xFFFF;

        while (*((unsigned short*)calculated_base) != 0x5A4D) {
            calculated_base -= 0x1000;
        }

        g_ModuleBase = (HINSTANCE)calculated_base;

        if (!_CRT_INIT(g_ModuleBase, DLL_PROCESS_ATTACH, NULL)) {
            return;
        }

        if (!LOG_INIT("C:\\Users\\Public\\algis.log")) {
            return;
        }

        ILOG("algis lock started\n");

        HANDLE hThread = CreateThread(NULL, 0, test, NULL, 0, NULL);
        if (hThread) {
            CloseHandle(hThread);
            ILOG("created thread!\n");
        }
    }
    return;
}
