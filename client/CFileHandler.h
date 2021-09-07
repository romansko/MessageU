/**
 * MessageU Client
 * @file CFileHandler.h
 * @brief Handle files on filesystem.
 * @author Roman Koifman
 */

#pragma once
#include <string>

class CFileHandler
{
public:
    bool fileOpen(const std::string& filepath, std::fstream& fs, bool write=false);
    bool fileClose(std::fstream& fs);
    bool fileRead(std::fstream& fs, uint8_t* const file, uint32_t bytes);
    bool fileWrite(std::fstream& fs, const uint8_t* const file, const uint32_t bytes);
    bool fileExists(const std::string& filepath);
    bool fileRemove(const std::string& filepath);
    uint32_t fileSize(std::fstream& fs);
};

