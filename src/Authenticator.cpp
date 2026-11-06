#include "Authenticator.h"

#include <iomanip>
#include <openssl/md5.h>
#include <random>
#include <sstream>
uint64_t Authenticator::generateSalt()
{
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dis;
    return dis(gen);
}
std::string Authenticator::formatSalt16(uint64_t salt)
{
    std::stringstream ss;
    ss << std::hex << std::uppercase << std::setfill('0') << std::setw(16) << salt;
    return ss.str();
}
std::string Authenticator::computeMD5Hash(std::string salt16, std::string password)
{
    std::string data = salt16 + password;
    unsigned char digest[MD5_DIGEST_LENGTH];
    MD5(reinterpret_cast<const unsigned char*>(data.c_str()), data.length(), digest);
    std::stringstream ss;
    for(int i = 0; i < MD5_DIGEST_LENGTH; ++i) {
        ss << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << (int)digest[i];
    }
    return ss.str();
}
bool Authenticator::verifyHash(std::string salt16, std::string password, std::string storedHash)
{
    std::string computed = computeMD5Hash(salt16, password);
    return computed == storedHash;
}