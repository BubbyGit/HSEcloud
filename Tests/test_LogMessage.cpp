#include <cassert>
#include <fstream>
#include <iostream>
#include "functions.h"

void logMessage(const std::string& message) {
    std::ofstream logFile(BASE_PATH + "/bot.log", std::ios_base::app);
    std::time_t now = std::time(nullptr);
    logFile << std::ctime(&now) << ": " << message << std::endl;
}