#pragma once
#include <cstdint>
#include <vector>
class VectorProcessor
{
private:
    bool readExact(int sockfd, void* buf, std::size_t len);
    bool writeExact(int sockfd, const void* buf, std::size_t len);

public:
    std::vector<std::vector<uint16_t>> receiveVectors(int sockfd);
    void sendResult(int sockfd, uint16_t result);
    uint16_t average(std::vector<uint16_t> vec);
    uint16_t saturate(uint32_t value);
};