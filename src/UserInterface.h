#pragma once
#include <string>
class UserInterface
{
private:
    std::string dbFile;
    std::string logFile;
    int port;

public:
    UserInterface();
    void readArgs(int argc, char** argv);
    void showHelp();
    std::string getDbFile() const;
    std::string getLogFile() const;
    int getPort() const;
};