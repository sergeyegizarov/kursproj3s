#pragma once
#include "Authenticator.h"
#include "ClientDB.h"
#include "Logger.h"
#include "UserInterface.h"
#include "VectorProcessor.h"

#include <string>
class Server
{
private:
    int port;
    ClientDB clientDB;
    Logger logFile;
    UserInterface ui;
    Authenticator auth;
    VectorProcessor processor;
    int serverSocket;
    bool readLine(int sockfd, std::string& out);
    bool writeLine(int sockfd, const std::string& msg);

public:
    Server(UserInterface& ui);
    void start();
    void acceptConnection();
};