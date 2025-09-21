#include <iostream>

#include <madoc/log_utils.h>


void logError(const std::string& module, const std::string& message) {
    std::cerr << "[ERROR] [" << module << "] " << message << std::endl;
}

void logWarning(const std::string& module, const std::string& message) {
    std::cerr << "[WARNING] [" << module << "] " << message << std::endl;
}

void logInfo(const std::string& module, const std::string& message) {
    std::cerr << "[INFO] [" << module << "] " << message << std::endl;
}
