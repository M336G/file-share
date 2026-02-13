#pragma once

class Helper {
public:
    static bool containsUnallowedPathCharacters(std::string path) {
        return path.find("..") != std::string::npos ||
            path.find("/") != std::string::npos ||
            path.find("\\") != std::string::npos;
    }
};