#pragma once
#include <map>
#include <string>
class ClientDB
{
private:
    std::map<std::string, std::string> users;
    std::string dbFile;

public:
    ClientDB(std::string file);
    bool load();
    std::string getPassword(std::string login);
    bool userExists(std::string login);
};