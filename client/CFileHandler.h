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
	CFileHandler();
    ~CFileHandler();

	// do not allow
    CFileHandler(const CFileHandler& other) = delete;
    CFileHandler(CFileHandler&& other) noexcept = delete;
    CFileHandler& operator=(const CFileHandler& other) = delete;
    CFileHandler& operator=(CFileHandler&& other) noexcept = delete;

	// file wrapper functions
    bool open(const std::string& filepath, bool write = false);
    void close();
    bool read(uint8_t* const dest, uint32_t bytes) const;
    bool write(const uint8_t* const src, const uint32_t bytes) const;
    bool fileExists(const std::string& filepath);
    bool remove(const std::string& filepath) const;
    bool readLine(std::string& line) const;
    uint32_t size() const;

private:
    std::fstream* _fileStream;
};

