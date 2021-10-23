/**
 * MessageU Client
 * @file AESWrapper.h
 * @brief Handle symmetric encryption. 
 * File was given by lecturers. Redundant code was removed for this project.
 * https://github.com/Romansko/MessageU/blob/main/client/header/AESWrapper.h
 */
#pragma once
#include <string>
#include "protocol.h"

class AESWrapper
{
public:
	static void GenerateKey(uint8_t* const buffer, const size_t length);

	AESWrapper();
	AESWrapper(const SSymmetricKey& symKey);
	
	virtual ~AESWrapper()                              = default;
	AESWrapper(const AESWrapper& other)                = delete;
	AESWrapper(AESWrapper&& other) noexcept            = delete;
	AESWrapper& operator=(const AESWrapper& other)     = delete;
	AESWrapper& operator=(AESWrapper&& other) noexcept = delete;

	SSymmetricKey getKey() const { return _key; }
	
	std::string encrypt(const std::string& plain) const;
	std::string encrypt(const uint8_t* plain,  size_t length) const;
	std::string decrypt(const uint8_t* cipher, size_t length) const;

private:
	SSymmetricKey _key;
};
