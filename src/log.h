#pragma once
#include <iostream>

// Helper functions to facilitate logging
class Log {
public:
    static void message(const std::string& message) {
        std::cout << message << std::endl;
    }

    static void error(const std::string& message) {
        std::cerr << message << std::endl;
    }
};