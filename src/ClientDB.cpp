#include "ClientDB.h"

#include <fstream>
#include <sstream>
ClientDB::ClientDB(std::string file)
    : dbFile(file)
{
}
bool ClientDB::load()
{
    std::ifstream file(dbFile);
    if(!file.is_open())
        return false;
    std::string line;
    while(std::getline(file, line)) {
        std::stringstream ss(line);
        std::string login, password;
        if(std::getline(ss, login, ':') && std::getline(ss, password)) {
            users[login] = password;
        }
    }
    return true;
}
std::string ClientDB::getPassword(std::string login)
{
    if(users.count(login))
        return users[login];
    return "";
}
bool ClientDB::userExists(std::string login) { return users.count(login) > 0; }
