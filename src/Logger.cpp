#include "Logger.h"

#include <ctime>
#include <iostream>
Logger::Logger(std::string file)
    : logFile(file)
{
    fileStream.open(logFile, std::ios::app);
    if(!fileStream.is_open()) {
        std::cerr << "Failed to open log file: " << logFile << "\n";
    }
}
Logger::~Logger()
{
    if(fileStream.is_open())
        fileStream.close();
}
void Logger::log(std::string message, Severity severity)
{
    if(!fileStream.is_open())
        return;
    std::time_t t = std::time(nullptr);
    char timeStr[100];
    std::strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", std::localtime(&t));
    std::string sevStr = "INFO";
    if(severity == Severity::WARNING)
        sevStr = "WARNING";
    else if(severity == Severity::CRITICAL)
        sevStr = "CRITICAL";
    fileStream << "[" << timeStr << "] [" << sevStr << "] " << message << "\n";
    fileStream.flush();
}