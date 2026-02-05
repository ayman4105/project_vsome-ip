#include <iostream>
#include <thread>
#include <chrono>
#include <fstream>

#include <CommonAPI/CommonAPI.hpp>
#include "v1/ayman/examples/SendFileProxy.hpp"

uint64_t localTimestamp = 0;
uint32_t localVersion = 0;

int main() {
    auto runtime = CommonAPI::Runtime::get();
    if (!runtime) return 1;

    std::string domain = "local";
    std::string instance = "ayman.examples.SendFile";

    auto proxy =
        runtime->buildProxy<v1::ayman::examples::SendFileProxy>(domain, instance);

    while (!proxy->isAvailable()) {
        std::cout << "[Client] Waiting for service...\n";
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    std::cout << "[Client] Service available\n";

    proxy->getNewDataAvailableEvent().subscribe(
        [&](uint32_t version, uint64_t timestamp) {
            std::cout << "[Client] Event received version=" << version
                      << ", timestamp=" << timestamp << std::endl;

            // Ignore older events
            if (timestamp <= localTimestamp)
                return;

            // Async request download
            proxy->RequestDownloadFileAsync(
                version,
                localTimestamp,
                [&, version, timestamp](const CommonAPI::CallStatus& status,
                                        bool accepted,
                                        uint64_t totalBytes,
                                        uint32_t chunkSize,
                                        uint64_t serverTimestamp) {

                    if (!accepted || status != CommonAPI::CallStatus::SUCCESS) {
                        std::cerr << "[Client] Download rejected or failed\n";
                        return;
                    }

                    std::ofstream out("received.txt", std::ios::binary);
                    if (!out.is_open()) {
                        std::cerr << "[Client] Failed to open output file\n";
                        return;
                    }

                    uint32_t totalChunks = (totalBytes + chunkSize - 1) / chunkSize;

                    // SYNC call for each chunk
                    for (uint32_t i = 0; i < totalChunks; i++) {
                        CommonAPI::CallStatus chunkStatus;
                        std::string data;
                        bool isLastChunk;
                        uint32_t returnedIndex;

                        proxy->RequestData(i, version,
                                           chunkStatus,
                                           data,
                                           isLastChunk,
                                           returnedIndex);

                        if (chunkStatus != CommonAPI::CallStatus::SUCCESS) {
                            std::cerr << "[Client] Failed to get chunk " << i << "\n";
                            break;
                        }

                        out.write(data.data(), data.size());
                        std::cout << "[Client] Received chunk " << i
                                  << ", size=" << data.size() << "\n";
                    }

                    out.close();
                    localTimestamp = serverTimestamp;
                    localVersion = version;

                    std::cout << "[Client] Download completed, version=" << version
                              << ", timestamp=" << timestamp << "\n";
                });
        });

    while (true)
        std::this_thread::sleep_for(std::chrono::seconds(1));
}
