/**
 * MessageU Client
 * @file AESWrapper.cpp
 * @brief Handle symmetric encryption.
 * File was given by lecturers. Redundant code was removed for this project.
 * https://github.com/Romansko/MessageU/blob/main/client/src/AESWrapper.cpp
 */
#include "AESWrapper.h"
#include <modes.h>
#include <aes.h>
#include <filters.h>
#include <stdexcept>
#include <immintrin.h>	// _rdrand32_step


void AESWrapper::GenerateKey(uint8_t* const buffer, const size_t length)
{
	for (size_t i = 0; i < length; i += sizeof(size_t))
		_rdrand32_step(reinterpret_cast<size_t*>(&buffer[i]));
}

AESWrapper::AESWrapper()
{
	GenerateKey(_key.symmetricKey, sizeof(_key.symmetricKey));
}


AESWrapper::AESWrapper(const SSymmetricKey& symKey) : _key(symKey)
{
}

std::string AESWrapper::encrypt(const std::string& plain) const
{
	return encrypt(reinterpret_cast<const uint8_t*>(plain.c_str()), plain.size());
}

std::string AESWrapper::encrypt(const uint8_t* plain, size_t length) const
{
	CryptoPP::byte iv[CryptoPP::AES::BLOCKSIZE] = { 0 };	// for practical use iv should never be a fixed value!

	CryptoPP::AES::Encryption aesEncryption(_key.symmetricKey, sizeof(_key.symmetricKey));
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

	CryptoPP::AES::Decryption aesDecryption(_key.symmetricKey, sizeof(_key.symmetricKey));
	CryptoPP::CBC_Mode_ExternalCipher::Decryption cbcDecryption(aesDecryption, iv);

	std::string decrypted;
	CryptoPP::StreamTransformationFilter stfDecryptor(cbcDecryption, new CryptoPP::StringSink(decrypted));
	stfDecryptor.Put(cipher, length);
	stfDecryptor.MessageEnd();

	return decrypted;
}
