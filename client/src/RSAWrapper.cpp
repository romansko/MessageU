#include "RSAWrapper.h"
#include "protocol.h"


RSAPublicWrapper::RSAPublicWrapper(const SPublicKey& publicKey)
{
	CryptoPP::StringSource ss((publicKey.publicKey), sizeof(publicKey.publicKey), true);
	_publicKey.Load(ss);
}

RSAPublicWrapper::RSAPublicWrapper(const uint8_t* key, size_t length)
{
	CryptoPP::StringSource ss((key), length, true);
	_publicKey.Load(ss);
}

RSAPublicWrapper::RSAPublicWrapper(const std::string& key)
{
	CryptoPP::StringSource ss(key, true);
	_publicKey.Load(ss);
}

std::string RSAPublicWrapper::getPublicKey() const
{
	std::string key;
	CryptoPP::StringSink ss(key);
	_publicKey.Save(ss);
	return key;
}

uint8_t* RSAPublicWrapper::getPublicKey(uint8_t* keyout, size_t length) const
{
	CryptoPP::ArraySink as((keyout), length);
	_publicKey.Save(as);
	return keyout;
}

std::string RSAPublicWrapper::encrypt(const std::string& plain)
{
	std::string cipher;
	CryptoPP::RSAES_OAEP_SHA_Encryptor e(_publicKey);
	CryptoPP::StringSource ss(plain, true, new CryptoPP::PK_EncryptorFilter(_rng, e, new CryptoPP::StringSink(cipher)));
	return cipher;
}

std::string RSAPublicWrapper::encrypt(const uint8_t* plain, size_t length)
{
	std::string cipher;
	CryptoPP::RSAES_OAEP_SHA_Encryptor e(_publicKey);
	CryptoPP::StringSource ss(plain, length, true, new CryptoPP::PK_EncryptorFilter(_rng, e, new CryptoPP::StringSink(cipher)));
	return cipher;
}



RSAPrivateWrapper::RSAPrivateWrapper()
{
	_privateKey.Initialize(_rng, BITS);
}

RSAPrivateWrapper::RSAPrivateWrapper(const uint8_t* key, size_t length)
{
	CryptoPP::StringSource ss((key), length, true);
	_privateKey.Load(ss);
}

RSAPrivateWrapper::RSAPrivateWrapper(const std::string& key)
{
	CryptoPP::StringSource ss(key, true);
	_privateKey.Load(ss);
}

std::string RSAPrivateWrapper::getPrivateKey() const
{
	std::string key;
	CryptoPP::StringSink ss(key);
	_privateKey.Save(ss);
	return key;
}

uint8_t* RSAPrivateWrapper::getPrivateKey(uint8_t* keyout, size_t length) const
{
	CryptoPP::ArraySink as((keyout), length);
	_privateKey.Save(as);
	return keyout;
}

std::string RSAPrivateWrapper::getPublicKey() const
{
	CryptoPP::RSAFunction publicKey(_privateKey);
	std::string key;
	CryptoPP::StringSink ss(key);
	publicKey.Save(ss);
	return key;
}

uint8_t* RSAPrivateWrapper::getPublicKey(uint8_t* keyout, size_t length) const
{
	CryptoPP::RSAFunction publicKey(_privateKey);
	CryptoPP::ArraySink as((keyout), length);
	publicKey.Save(as);
	return keyout;
}

std::string RSAPrivateWrapper::decrypt(const std::string& cipher)
{
	std::string decrypted;
	CryptoPP::RSAES_OAEP_SHA_Decryptor d(_privateKey);
	CryptoPP::StringSource ss_cipher(cipher, true, new CryptoPP::PK_DecryptorFilter(_rng, d, new CryptoPP::StringSink(decrypted)));
	return decrypted;
}

std::string RSAPrivateWrapper::decrypt(const uint8_t* cipher, size_t length)
{
	std::string decrypted;
	CryptoPP::RSAES_OAEP_SHA_Decryptor d(_privateKey);
	CryptoPP::StringSource ss_cipher((cipher), length, true, new CryptoPP::PK_DecryptorFilter(_rng, d, new CryptoPP::StringSink(decrypted)));
	return decrypted;
}
