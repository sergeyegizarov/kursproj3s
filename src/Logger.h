#pragma once
#include <fstream>
#include <string>
enum class Severity { INFO, WARNING, CRITICAL };
class Logger
{
private:
    std::string logFile;
    std::ofstream fileStream;

public:
    Logger(std::string file);
    ~Logger();
    void log(std::string message, Severity severity);
};