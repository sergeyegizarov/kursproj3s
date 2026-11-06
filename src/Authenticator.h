#pragma once
#include <cstdint>
#include <string>
class Authenticator
{
public:
    uint64_t generateSalt();
    std::string formatSalt16(uint64_t salt);
    std::string computeMD5Hash(std::string salt16, std::string password);
    bool verifyHash(std::string salt16, std::string password, std::string storedHash);
};