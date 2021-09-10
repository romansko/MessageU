/**
 * MessageU Client
 * @file CFileHandler.h
 * @brief Handle files on filesystem.
 * @author Roman Koifman
 */

#pragma once
#include <string>
#include <fstream>

class CFileHandler
{
public:
    bool fileOpen(const std::string& filepath, std::fstream& fs, bool write=false);
    bool fileClose(std::fstream& fs);
    bool fileRead(std::fstream& fs, uint8_t* const dest, uint32_t bytes);
    bool fileWrite(std::fstream& fs, const uint8_t* const src, const uint32_t bytes);
    bool fileExists(const std::string& filepath);
    bool fileRemove(const std::string& filepath);
    bool fileReadLine(std::fstream& fs, std::string& line);
    uint32_t fileSize(std::fstream& fs);
};

