#pragma once

#include <string>
#include "protocol.h"

struct SSymmetricKey;

class AESWrapper
{
public:

	static const size_t DEFAULT_KEYLENGTH = CLIENT_ID_SIZE;
private:
	uint8_t _key[DEFAULT_KEYLENGTH];
	
public:
	static uint8_t* GenerateKey(uint8_t* buffer, size_t length);

	AESWrapper();
	AESWrapper(SSymmetricKey& symKey);
	AESWrapper(const uint8_t* key, size_t size);
	
	virtual ~AESWrapper() = default;
	AESWrapper(const AESWrapper& other) = delete;
	AESWrapper(AESWrapper&& other) noexcept = delete;
	AESWrapper& operator=(const AESWrapper& other) = delete;
	AESWrapper& operator=(AESWrapper&& other) noexcept = delete;

	const uint8_t* getKey() const;

	std::string encrypt(const std::string& plain) const;
	std::string encrypt(const uint8_t* plain,  size_t length) const;
	std::string decrypt(const uint8_t* cipher, size_t length) const;
};
