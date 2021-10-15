#include "AESWrapper.h"

#include <modes.h>
#include <aes.h>
#include <filters.h>

#include <stdexcept>
#include <immintrin.h>	// _rdrand32_step


uint8_t* AESWrapper::GenerateKey(uint8_t* buffer, size_t length)
{
	for (size_t i = 0; i < length; i += sizeof(size_t))
		_rdrand32_step(reinterpret_cast<size_t*>(&buffer[i]));
	return buffer;
}

AESWrapper::AESWrapper()
{
	GenerateKey(_key, DEFAULT_KEYLENGTH);
}

AESWrapper::AESWrapper(const uint8_t* key, size_t length)
{
	if (length != DEFAULT_KEYLENGTH)
		throw std::length_error("key length must be 16 bytes");
	memcpy_s(_key, DEFAULT_KEYLENGTH, key, length);
}

AESWrapper::~AESWrapper()
{
}

const uint8_t* AESWrapper::getKey() const 
{ 
	return _key; 
}

std::string AESWrapper::encrypt(const uint8_t* plain, size_t length) const
{
	CryptoPP::byte iv[CryptoPP::AES::BLOCKSIZE] = { 0 };	// for practical use iv should never be a fixed value!

	CryptoPP::AES::Encryption aesEncryption(_key, DEFAULT_KEYLENGTH);
	CryptoPP::CBC_Mode_ExternalCipher::Encryption cbcEncryption(aesEncryption, iv);

	std::string cipher;
	CryptoPP::StreamTransformationFilter stfEncryptor(cbcEncryption, new CryptoPP::StringSink(cipher));
	stfEncryptor.Put(plain, length);
	stfEncryptor.MessageEnd();

	return cipher;
}


std::string AESWrapper::decrypt(const uint8_t* cipher, size_t length) const
{
	CryptoPP::byte iv[CryptoPP::AES::BLOCKSIZE] = { 0 };	// for practical use iv should never be a fixed value!

	CryptoPP::AES::Decryption aesDecryption(_key, DEFAULT_KEYLENGTH);
	CryptoPP::CBC_Mode_ExternalCipher::Decryption cbcDecryption(aesDecryption, iv);

	std::string decrypted;
	CryptoPP::StreamTransformationFilter stfDecryptor(cbcDecryption, new CryptoPP::StringSink(decrypted));
	stfDecryptor.Put(cipher, length);
	stfDecryptor.MessageEnd();

	return decrypted;
}
