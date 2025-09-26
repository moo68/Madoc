#pragma once

#include <string>

// Prints out formatted and standardized errors, warnings, and info
void logError(const std::string& module, const std::string& message);
void logWarning(const std::string& module, const std::string& message);
void logInfo(const std::string& module, const std::string& message);
