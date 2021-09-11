/**
 * MessageU Client
 * @file CFileHandler.cpp
 * @brief Handle files on filesystem.
 * @author Roman Koifman
 */

#include "CFileHandler.h"
#include <fstream>
#include <boost/filesystem.hpp>  // for create_directories


/**
 * Open a file for read/write. Create folders in filepath if do not exist.
 * Relative paths not supported!
 */
bool CFileHandler::fileOpen(const std::string& filepath, std::fstream& fs, bool write)
{
	try
	{
		if (filepath.empty())
			return false;
		// create directories within the path if they are do not exist.
		const auto parent = boost::filesystem::path(filepath).parent_path();
		if (!parent.empty())
		{
			(void)create_directories(parent);
		}
		const auto flags = write ? (std::fstream::binary | std::fstream::out) : (std::fstream::binary | std::fstream::in);
		fs.open(filepath, flags);
		return fs.is_open();
	}
	catch (...)
	{
		return false;
	}
}


/**
 * Close file stream.
 */
void CFileHandler::fileClose(std::fstream& fs)
{
	try
	{
		fs.close();
	}
	catch (...)
	{
		/* Do Nothing */
	}
}

/**
 * Read bytes from fs to dest.
 */
bool CFileHandler::fileRead(std::fstream& fs, uint8_t* const dest, uint32_t bytes)
{
	try
	{
		if (dest == nullptr || bytes == 0)
			return false;
		fs.read(reinterpret_cast<char*>(dest), bytes);
		return true;
	}
	catch (...)
	{
		return false;
	}
}


/**
 * Write given bytes from src to fs.
 */
bool CFileHandler::fileWrite(std::fstream& fs, const uint8_t* const src, const uint32_t bytes)
{
	try
	{
		if (src == nullptr || bytes == 0)
			return false;
		fs.write(reinterpret_cast<const char*>(src), bytes);
		return true;
	}
	catch (...)
	{
		return false;
	}
}



/**
 * Check whether a file exists given a filePath.
 */
bool CFileHandler::fileExists(const std::string& filePath)
{
	if (filePath.empty())
		return false;

	try
	{
		const std::ifstream fs(filePath);
		return (!fs.fail());
	}
	catch (...)
	{
		return false;
	}
}



/**
 * Removes a file given a filePath.
 */
bool CFileHandler::fileRemove(const std::string& filePath)
{
	try
	{
		return (0 == std::remove(filePath.c_str()));   // 0 upon success..
	}
	catch (...)
	{
		return false;
	}
}


/**
 * Read a single line from fs to line.
 */
bool CFileHandler::fileReadLine(std::fstream& fs, std::string& line)
{
	try
	{
		std::getline(fs, line);
		return true;
	}
	catch (...)
	{
		return false;
	}
}


/**
 * Calculate the file size which is opened by fs.
 */
uint32_t CFileHandler::fileSize(std::fstream& fs)
{
	try
	{
		const auto cur = fs.tellg();
		fs.seekg(0, std::fstream::end);
		const auto size = fs.tellg();
		if ((size <= 0) || (size > UINT32_MAX))    // do not support more than uint32 max size files. (up to 4GB).
			return 0;
		fs.seekg(cur);    // restore position
		return static_cast<uint32_t>(size);
	}
	catch (...)
	{
		return 0;
	}
}