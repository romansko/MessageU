#pragma once

#include <string>
#include "protocol.h"

class AESWrapper
{
public:
	static const size_t DEFAULT_KEYLENGTH = CLIENT_ID_SIZE;
private:
	uint8_t _key[DEFAULT_KEYLENGTH];
	AESWrapper(const AESWrapper& aes);
public:
	static uint8_t* GenerateKey(uint8_t* buffer, size_t length);

	AESWrapper();
	AESWrapper(const uint8_t* key, size_t size);
	~AESWrapper();

	const uint8_t* getKey() const;

	std::string encrypt(const uint8_t* plain,  size_t length) const;
	std::string decrypt(const uint8_t* cipher, size_t length) const;
};
