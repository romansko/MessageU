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
   @brief open a file for read/write. Create folders in filepath if do not exist.
   @param filepath the file's filepath to open.
   @param fs file stream which will be opened with the filepath.
   @param write open file for writing?
   @return true if opened successfully. false otherwise.
 */
bool CFileHandler::fileOpen(const std::string& filepath, std::fstream& fs, bool write)
{
	try
	{
		if (filepath.empty())
			return false;
		// create directories within the path if they are do not exist.
		(void)create_directories(boost::filesystem::path(filepath).parent_path());
		const auto flags = write ? (std::fstream::binary | std::fstream::out) : (std::fstream::binary | std::fstream::in);
		fs.open(filepath, flags);
		return fs.is_open();
	}
	catch (std::exception&)
	{
		return false;
	}
}


/**
   @brief close file stream.
   @param fs file stream which will be closed.
   @return true if closed successfully. false otherwise.
 */
bool CFileHandler::fileClose(std::fstream& fs)
{
	try
	{
		fs.close();
		return true;
	}
	catch (std::exception&)
	{
		return false;
	}
}

/**
   @brief read bytes from fs to file.
   @param fs opened file stream to read from.
   @param file source to copy the data to.
   @param bytes bytes to read.
   @return true if read successfully. false, otherwise.
 */
bool CFileHandler::fileRead(std::fstream& fs, uint8_t* const file, uint32_t bytes)
{
	try
	{
		if (file == nullptr || bytes == 0)
			return false;
		fs.read(reinterpret_cast<char*>(file), bytes);
		return true;
	}
	catch (std::exception&)
	{
		return false;
	}
}


/**
   @brief write bytes from fs to file.
   @param fs Opened file stream to write from.
   @param file the file to read from.
   @param bytes bytes to write.
   @return true upon successful write. false otherwise.
 */
bool CFileHandler::fileWrite(std::fstream& fs, const uint8_t* const file, const uint32_t bytes)
{
	try
	{
		if (file == nullptr || bytes == 0)
			return false;
		fs.write(reinterpret_cast<const char*>(file), bytes);
		return true;
	}
	catch (std::exception&)
	{
		return false;
	}
}



/**
   @brief Check if file exists given a file path.
   @param filePath the file's filepath.
   @return true if file exists.
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
	catch (std::exception&)
	{
		return false;
	}
}



/**
   @brief Removes a file given a file path.
   @param filePath the file's filepath to remove.
   @return true if successfully removed the file. False, otherwise.
		   If file not exists, return false.
 */
bool CFileHandler::fileRemove(const std::string& filePath)
{
	try
	{
		return (0 == std::remove(filePath.c_str()));   // 0 upon success..
	}
	catch (std::exception&)
	{
		return false;
	}
}


/**
   @brief calculate file size which is opened by fs.
   @param fs opened file stream to read from.
   @return file's size. 0 if failed.
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
	catch (std::exception&)
	{
		return 0;
	}
}