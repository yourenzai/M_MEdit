#pragma once

#include <fstream>
#include <filesystem>
#include <iostream>
#include <string>

//saveFile
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

//readFile
inline bool readFile(
    const std::filesystem::path& filePath,
    std::string& outContent)
{
    outContent.clear();

    std::ifstream file(filePath,std::ios::binary);
    if (!file.is_open())
    {
        std::cerr << "Failed to open file for reading:" <<filePath.string() << std::endl;
        return false;
    }

    outContent.assign(
        std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>());

    if (file.bad())
    {
        std::cerr << "Failed while reading file: " << filePath.string() << std::endl;
        outContent.clear();
        return false;
    }

    if (outContent.size() >= 3 &&
        static_cast<unsigned char>(outContent[0]) == 0xEF &&
        static_cast<unsigned char>(outContent[1]) == 0xBB &&
        static_cast<unsigned char>(outContent[2]) == 0xBF)
    {
        outContent.erase(0,3);
    }
    
    return true;
}