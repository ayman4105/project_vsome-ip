#include <iostream>
#include <thread>
#include <chrono>

#include <CommonAPI/CommonAPI.hpp>
#include "SendFileImplstub.hpp"

int main() {
    auto runtime = CommonAPI::Runtime::get();
    if (!runtime) return 1;

    std::string domain = "local";
    std::string instance = "ayman.examples.SendFile";

    auto stub = std::make_shared<SendFileImplStub>("/home/ayman/ITI/fady/project_vsome-ip/data.txt");

    if (!runtime->registerService(domain, instance, stub)) {
        std::cerr << "[Server] Failed to register service\n";
        return 1;
    }

    std::cout << "[Server] Service running...\n";

    while (true) {
        stub->checkForFileChange();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}
