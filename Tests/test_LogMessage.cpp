#include <cassert>
#include <fstream>
#include <iostream>
#include "functions.h"

void testLogMessage() {
    std::string testMessage = "Test log message";
    logMessage(testMessage);

    std::ifstream logFile(getCurrentDir() + "/bot.log");
    std::
