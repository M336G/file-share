#pragma once
#include <string>

// This is where all of the server's configuration will be put
class Config {
public:
    inline static uint16_t PORT = 8929;
    inline static std::string STORAGE_DIRECTORY;
    inline static bool DISABLE_INDEX = false;

    static void loadEnvironmentVariables();
};