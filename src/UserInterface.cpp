#include "UserInterface.h"

#include <cstdlib>
#include <iostream>
UserInterface::UserInterface()
    : dbFile("/home/25pi108@ibst.psu/vcalc.conf")
    , logFile("/home/25pi108@ibst.psu/vcalc.log")
    , port(33333)
{
}
void UserInterface::readArgs(int argc, char** argv)
{
    if(argc == 1) {
        showHelp();
    }
    for(int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if(arg == "-h" || arg == "--help") {
            showHelp();
            exit(0);
        } else if(arg == "--db" && i + 1 < argc) {
            dbFile = argv[++i];
        } else if(arg == "--log" && i + 1 < argc) {
            logFile = argv[++i];
        } else if(arg == "--port" && i + 1 < argc) {
            port = std::stoi(argv[++i]);
        }
    }
}
void UserInterface::showHelp()
{
    std::cout << "Usage: server [options]\n"
              << "Options:\n"
              << " --db <file> Client database file (default: " << dbFile << ")\n"
              << " --log <file> Log file (default: " << logFile << ")\n"
              << " --port <num> Server port (default: " << port << ")\n"
              << " -h, --help Show this help\n";
}
std::string UserInterface::getDbFile() const { return dbFile; }
std::string UserInterface::getLogFile() const { return logFile; }
int UserInterface::getPort() const { return port; }