#define ALGIS_EXTERNAL
#include "../vendor/algis.h"


// I would recomend to do "using namespace algis"
/*
int main() {

	void* QPC = GetProcAddress(GetModuleHandleA("Kernel32.dll"), "QueryPerformanceCounter");
	A_HOOK qpcHook = nullptr;

	printf("starting test 1\n");

	if (algis::hk_init("RobloxPlayerBeta.exe") != ALGIS_OK) {
		printf("IHKInit failed. is the target process running?");
		return 1;
	}
	printf("test 1 passed, test 2\n");
	uintptr_t entryPoint = 0;
	const wchar_t* dllPath = L"C:\\Users\\cheese\\source\\repos\\algis\\x64\\Release\\exthk.dll";

	if (algis::map(dllPath, &entryPoint) != ALGIS_OK){
		printf("IMap failed\n");
		algis::hk_uninit();
		return 1;
	}

	printf("test 2 passed, test 3\n");
	ALGIS_STATUS status = algis::create_hook(QPC, (void*)entryPoint, &qpcHook);
	if (status != ALGIS_OK) {
		printf("ICreateHook failed\n");
		algis::hk_uninit();
	}
	printf("hook placed ad address: 0x%p\n", (void*)entryPoint);
	printf("Press any key to clean up.\n");
	Sleep(1000);
	algis::destroy_hook(qpcHook);
	system("pause");

	algis::hk_uninit();

	return 0;
} */

/*
int main() {
	std::cout << "server started\n";


	char buffer[256] = { 0 };
	DWORD bytesRead = 0;
	LPCWSTR pipeName = L"\\\\.\\pipe\\algis_data_pipe";

	// this will block until a sender connects and writes data
	ALGIS_STATUS status = algis::xrecieve(XPIPE, pipeName, buffer, sizeof(buffer) - 1, &bytesRead, 5000);

	if (status == ALGIS_OK) {
		std::cout << "received " << bytesRead << " bytes: \"" << buffer << "\"\n";
	}
	else {
		std::cerr << "failed to receive data. status: " << status << "\n";
	}

	std::cout << "press any key to exit...\n";
	std::getchar();
	return 0;
}  SERVER CODE WORKS */

int main() {
	if (!LOG_INIT("algis_runtime.log")) {
		return -1;
	}

	ILOG("info logging works!");
	WLOG("warning logging works!");
	ELOG("error logging works!");


	return 0;
}