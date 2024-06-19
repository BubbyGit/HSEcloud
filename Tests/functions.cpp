#include <string>
#include <fstream>
#include <ctime>
#include <filesystem>

namespace fs = std::filesystem;

std::string getCurrentDir() {
    return fs::current_path().string();
}

void logMessage(const std::string& message) {
    const std::string BASE_PATH = getCurrentDir();
    std::ofstream logFile(BASE_PATH + "/bot.log", std::ios_base::app);
    std::time_t now = std::time(nullptr);
    logFile << std::ctime(&now) << ": " << message << std::endl;
}
