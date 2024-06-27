#include <cassert>
#include <fstream>
#include <iostream>
#include "functions.h"

void testGetCurrentDir() {
    std::string dir = getCurrentDir();
    assert(!dir.empty());
    std::cout << "Current Directory: " << dir << std::endl;
}

void testLogMessage() {
    std::string testMessage = "Test log message";
    logMessage(testMessage);

    std::ifstream logFile(getCurrentDir() + "/bot.log");
    std::string lastLine;
    while (logFile.good()) {
        std::string line;
        std::getline(logFile, line);
        if (!line.empty()) {
            lastLine = line;
        }
    }
    logFile.close();

    assert(lastLine.find(testMessage) != std::string::npos);
    std::cout << "Log Message Test Passed" << std::endl;
}

int main(int argc, char **argv) {
    if (argc > 1) {
        std::string testName = argv[1];
        if (testName == "testGetCurrentDir") {
            testGetCurrentDir();
        } else if (testName == "testLogMessage") {
            testLogMessage();
        }
    } else {
        std::cerr << "No test specified." << std::endl;
        return 1;
    }
    return 0;
}
