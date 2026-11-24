#pragma once

#include <fstream>
#include <filesystem>
#include <iostream>
#include <string>

inline bool saveFile(
    const std::filesystem::path& filePath,
    const std::string& content)
{
    std::ofstream file(filePath, std::ios::binary);
    if (!file.is_open())
    {
        std::cerr << "Failed to open file for writing: " << filePath.string() << std::endl;
        return false;
    }
    file << content;
    return file.good();
}