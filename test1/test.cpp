#define ALGIS_EXTERNAL
#include "../algis/vendor/algis.h"

int main() {
    std::cout << "client started\n";

    std::string message = "hello from algis";
    LPCWSTR pipeName = L"\\\\.\\pipe\\algis_data_pipe";

    // Small pause to make sure the receiver is up and listening
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    std::cout << "sending data to pipe\n";
    ALGIS_STATUS status = algis::xsend(XPIPE, pipeName, message.c_str(), message.size() + 1);

    if (status == ALGIS_OK) {
        std::cout << "data sent successfully\n";
    }
    else {
        std::cerr << "send failed. status: " << status << "\n";
    }

    std::cout << "press any key to exit...\n";
    std::getchar();
	return 0;
}