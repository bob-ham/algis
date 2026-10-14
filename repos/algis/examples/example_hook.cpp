#define ALGIS_EXTERNAL
#include "../src/algis.h"

const char* szProcessName = "RobloxPlayerBeta.exe"; // example process
// example DLL (all this does is make a log file)
const wchar_t* szDLLPath = L"C\\Users\\cheese\\source\\repos\\ExternHook\\x64\\Release\\exthk.dll";

int main() {
  // use QPC has a hook
  void* QPC = GetProcAddress(GetModuleHandleA("Kernel32.dll"), "QueryPerformanceCounter");
  void* QPC_HOOK = nullptr;
  ALGIS_STATUS status = ALGIS_NULL_STATUS;

  status = algis::hook_init(szProcessName);
  if (status != ALGIS_OK) {
    printf("algis::hook_init failed, error: [%s]", algis::al_to_string(status));
    return -1;
  }

  // map dll into memory then hook
  std::uintptr_t entry = 0;
  status = algis::map(szDLLPath, &entry);
  if (status == ALGIS_OK) {
    if (entry == 0) {
      printf("entry point is 0\n");
      return -1;
    }

    // hook onto qpc
    status = algis::create_hook(QPC, (void*)entry, &QPC_HOOK);
    if (status != ALGIS_OK) {
      printf("algis::create_hook failed, error [%s]", algis::al_to_string(status));
      algis::destroy_hook(QPC_HOOK);
      return -1;
    }
  } else {
    printf("algis::map failed, error: [%s]", algis::al_to_string(status));
    return -1;
  }

  printf("hook placed aat addresss: [0x%p]\n", (void*)entry);
  Sleep(1000);
  algis::destroy_hook(QPC_HOOK);
  system("pause");

  algis::hook_uninit();

  return 0;
}