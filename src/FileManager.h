#pragma once

#include <fstream>
#include <iostream>

inline bool saveFile(
    const std::string& filePath,
    const std::string& content)
{
    std::ofstream file(filePath);
    if (!file.is_open())
    {
        std::cerr << "Failed to open file for writing: " << filePath << std::endl;
        return false;
    }
    file <<content;
    file.close();
    return true;
}