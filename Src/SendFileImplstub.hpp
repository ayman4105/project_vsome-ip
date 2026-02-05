#pragma once

#include <CommonAPI/CommonAPI.hpp>
#include "v1/ayman/examples/SendFileStubDefault.hpp"

#include <fstream>
#include <vector>
#include <filesystem>
#include <chrono>
#include <iostream>

class SendFileImplStub : public v1::ayman::examples::SendFileStubDefault {
private:
    std::string file_path;
    uint32_t data_version;
    uint64_t last_timestamp;
    std::streampos last_pos;

public:
    SendFileImplStub(const std::string& path)
        : file_path(path),
          data_version(1),
          last_pos(0)
    {
        last_timestamp = getFileTimestamp();
    }

    uint64_t getFileTimestamp() {
        namespace fs = std::filesystem;
        auto ftime = fs::last_write_time(file_path);
        return std::chrono::duration_cast<std::chrono::seconds>(
            ftime.time_since_epoch()
        ).count();
    }

    void checkForFileChange() {
        uint64_t current = getFileTimestamp();
        if (current != last_timestamp) {
            last_timestamp = current;
            data_version++;
            last_pos = 0;

            std::cout << "[Server] File changed! version="
                      << data_version << std::endl;

            fireNewDataAvailableEvent(data_version, last_timestamp);
        }
    }

    // ----------- Methods from FIDL -----------

    void RequestDownloadFile(
        const std::shared_ptr<CommonAPI::ClientId>,
        uint32_t clientVersion,
        uint64_t clientTimestamp,
        RequestDownloadFileReply_t reply) override
    {
        std::ifstream file(file_path, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            reply(false, 0, 0, last_timestamp);
            return;
        }

        uint64_t totalBytes = file.tellg();
        uint32_t chunkSize = 1024;

        std::cout << "[Server] Download accepted\n";
        reply(true, totalBytes, chunkSize, last_timestamp);
    }

    void RequestData(
        const std::shared_ptr<CommonAPI::ClientId>,
        uint32_t chunkIndex,
        uint32_t,
        RequestDataReply_t reply) override
    {
        std::ifstream file(file_path, std::ios::binary);
        if (!file.is_open()) {
            reply("", true, chunkIndex);
            return;
        }

        uint32_t chunkSize = 1024;
        file.seekg(chunkIndex * chunkSize);

        std::vector<char> buffer(chunkSize);
        file.read(buffer.data(), chunkSize);
        std::streamsize bytesRead = file.gcount();

        bool lastChunk = bytesRead < chunkSize;

        reply(std::string(buffer.data(), bytesRead), lastChunk, chunkIndex);
    }
};
