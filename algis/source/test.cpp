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
	const wchar_t* dllPath = L"C:\\Users\\cheese\\source\\repos\\algis\\x64\\Release\\algisdlltest.dll";

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
} 
*/

/*

void on_receive(void* data, size_t size) {
	std::string message(static_cast<char*>(data), size);
	std::cout << "\nrecieved " << size << " bytes: \"" << message << "\"\n";
}

int main() {
	std::cout << "server started\n";

	char buffer[256] = { 0 };
	DWORD bytesRead = 0;
	LPCWSTR pipeName = L"\\\\.\\pipe\\algis_data_pipe";

	// this will block until a sender connects and writes data
	ALGIS_STATUS status = algis::xrecieve(XPIPE, pipeName, nullptr, 0, nullptr, INFINITE, on_receive, IPC_THREAD_CREATE);

	if (status == ALGIS_OK) {
		printf("background thread spawned!\n");
	}
	else {
		std::cerr << "failed to receive data. status: " << status << "\n";
	}

	std::cout << "main thread is unblocked\n";
	std::getchar();
	return 0;
}  
*/


void reciver_thread(LPCWSTR ipcName) {
	std::wcout << L"starting and waiting for data on: " << ipcName << std::endl;

	char recieveBuffer[256] = { 0 };
	DWORD bytesRead = 0;

	ALGIS_STATUS status = algis::xrecieve(XSMEM, ipcName, recieveBuffer, sizeof(recieveBuffer) - 1, &bytesRead, 5000);

	if (status == ALGIS_OK) {
		std::cout << "success, received " << bytesRead << " bytes" << std::endl;
		std::cout << "data content: " << recieveBuffer << std::endl;
	}
	else {
		std::cerr << "failed with status code: " << status << std::endl;
	}
}

void sender_thread(LPCWSTR ipcName) {
	std::this_thread::sleep_for(std::chrono::milliseconds(500));

	std::string message = "algis shared memory test";
	std::wcout << L"sending message to: " << ipcName << std::endl;

	ALGIS_STATUS status = algis::xsend(XSMEM, ipcName, message.c_str(), message.size() + 1);

	if (status == ALGIS_OK) {
		std::cout << "data send succesfully\n";
	}
	else {
		std::cerr << "send failed with status code: " << status << std::endl;
	}
}

int main() {
	LPCWSTR IpcName = L"Local\\AlgisSharedMemory";

	std::cout << "shared memory test\n";

	std::thread receiver(reciver_thread, IpcName);
	std::thread sender(sender_thread, IpcName);

	receiver.join();
	sender.join();

	std::cout << "test completed\n";
	return 0;
}

/*
int main() {
	if (!LOG_INIT("algis_runtime.log")) {
		return -1;
	}

	ILOG("info logging works!");
	WLOG("warning logging works!");
	ELOG("error logging works!");


	return 0;
} */