#include "config.h"
#include "log.h"

#include <filesystem>
#include <fstream>

static std::string trim(std::string_view s) {
    size_t start = s.find_first_not_of(" \t");
    size_t end = s.find_last_not_of(" \t");
    if (start == std::string_view::npos) return std::string{};
    return std::string(s.substr(start, end - start + 1));
}

void Config::loadEnvironmentVariables() {
    /*
        Try getting the environments variables directly first
    */

    // Try getting the PORT
    if (const char *envPort = std::getenv("PORT")) {
        try {
            int portValue = std::stoi(envPort);
            if (portValue >= 0 && portValue <= 65535)
                PORT = static_cast<uint16_t>(portValue);
        } catch (...) {
            Log::error(std::format("Invalid PORT environment variable: {}", envPort));
        }
    }

    // Try getting the STORAGE_DIRECTORY
    if (const char *envDir = std::getenv("STORAGE_DIRECTORY"))
        STORAGE_DIRECTORY = envDir;

    // Try getting the DISABLE_INDEX value
    if (const char *envIndex = std::getenv("STORAGE_DIRECTORY")) {
        if (envIndex == "true" || envIndex == "1")
            DISABLE_INDEX = true;
        else
            DISABLE_INDEX = false;
    }
    /*
        Try getting the environment variables from .env afterwards
        (it will override the ones previously gotten if they were set)
    */

    // Check if .env even exists first
    if (std::filesystem::exists(".env")) {
        std::ifstream envFile(".env");
        std::string line;
        Log::message(line);

        while (std::getline(envFile, line)) {
            line = trim(line);

            // Skip empty lines or comments
            if (line.empty() || line[0] == '#')
                continue;

            // Split at '='
            auto pos = line.find('=');
            if (pos == std::string::npos) continue;

            auto key = trim(line.substr(0, pos));
            auto value = trim(line.substr(pos + 1));

            // Try getting the PORT
            if (key == "PORT") {
                try {
                    int portValue = std::stoi(value);
                    if (portValue >= 0 && portValue <= 65535)
                        PORT = static_cast<uint16_t>(portValue);
                } catch (...) {
                    Log::error(std::format("Invalid PORT value: {}", value));
                }
            }
            
            // Try getting the STORAGE_DIRECTORY
            else if (key == "STORAGE_DIRECTORY") {
                STORAGE_DIRECTORY = value;
            }

            // Try getting the DISABLE_INDEX value
            else if (key == "DISABLE_INDEX") {
                if (value == "true" || value == "1")
                    DISABLE_INDEX = true;
                else
                    DISABLE_INDEX = false;
            }
        }
    }
}