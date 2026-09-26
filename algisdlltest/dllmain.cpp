#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <vector>

#define ALGIS_INTERNAL
#include "../algis/vendor/algis.h"
#include "minhook-1.3.4/minhook-1.3.4/include/MinHook.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

extern "C" {
    extern BOOL WINAPI _CRT_INIT(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved);
}

HINSTANCE g_ModuleBase = nullptr;


HRESULT STDMETHODCALLTYPE hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags) {
    static bool loggedOnce = false;
    if (!loggedOnce) {
        ILOG("hkPresent is firing\n");
        loggedOnce = true;
    }
    typedef HRESULT(STDMETHODCALLTYPE* Present_t)(IDXGISwapChain*, UINT, UINT);
    auto oPresent = reinterpret_cast<Present_t>(algis::getVOrignal(&hkPresent));

    if (oPresent) {
        return oPresent(pSwapChain, SyncInterval, Flags);
    }
    return S_OK;
}

// 2. Initialize using your custom VMT framework
void InitPresentHk() {
    ILOG("Initializing custom shadow VMT hook for Present...\n");

    // Create a temporary dummy window & D3D11 device
    WNDCLASSEXW wc = { sizeof(WNDCLASSEXW), CS_CLASSDC, DefWindowProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, L"AlgisDummy", NULL };
    RegisterClassExW(&wc);
    HWND hWnd = CreateWindowW(L"AlgisDummy", L"", WS_OVERLAPPEDWINDOW, 100, 100, 300, 300, NULL, NULL, wc.hInstance, NULL);

    DXGI_SWAP_CHAIN_DESC sd = {};
    sd.BufferCount = 1;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;

    IDXGISwapChain* pDummySwapChain = nullptr;
    ID3D11Device* pDummyDevice = nullptr;
    D3D_FEATURE_LEVEL levels[] = { D3D_FEATURE_LEVEL_11_0 };

    if (D3D11CreateDeviceAndSwapChain(
        NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, levels, 1,
        D3D11_SDK_VERSION, &sd, &pDummySwapChain, &pDummyDevice, NULL, NULL) == S_OK)
    {
        // Pass the dummy swapchain instance and index 8 (Present) into your framework
        if (algis::createVHook(&hkPresent, pDummySwapChain, 8)) {
            if (algis::enableVHook(&hkPresent)) {
                ILOG("Custom shadow VMT Present hook enabled successfully!\n");
            }
            else {
                ILOG("Failed to enable custom VMT hook.\n");
            }
        }
        else {
            ILOG("Failed to create custom VMT hook entry.\n");
        }

        // Clean up dummy objects
        pDummySwapChain->Release();
        pDummyDevice->Release();
        DestroyWindow(hWnd);
        UnregisterClassW(L"AlgisDummy", wc.hInstance);
    }
    else {
        ILOG("Failed to create dummy device for shadow VMT hook.\n");
        DestroyWindow(hWnd);
        UnregisterClassW(L"AlgisDummy", wc.hInstance);
    }
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

        ILOG("started algis log\n");

        InitPresentHk();
        
    }
    return;
}
