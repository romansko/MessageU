#pragma once

#include <osrng.h>
#include <rsa.h>

#include <string>



class RSAPublicWrapper
{
public:
	static const size_t KEYSIZE = 160;
	static const size_t BITS = 1024;

private:
	CryptoPP::AutoSeededRandomPool _rng;
	CryptoPP::RSA::PublicKey _publicKey;

	RSAPublicWrapper(const RSAPublicWrapper& rsapublic);
	RSAPublicWrapper& operator=(const RSAPublicWrapper& rsapublic);
public:

	RSAPublicWrapper(const uint8_t* key, size_t length);
	RSAPublicWrapper(const std::string& key);
	~RSAPublicWrapper();

	std::string getPublicKey() const;
	uint8_t* getPublicKey(uint8_t* keyout, size_t length) const;

	std::string encrypt(const std::string& plain);
	std::string encrypt(const uint8_t* plain, size_t length);
};


class RSAPrivateWrapper
{
public:
	static const size_t BITS = 1024;

private:
	CryptoPP::AutoSeededRandomPool _rng;
	CryptoPP::RSA::PrivateKey _privateKey;

	RSAPrivateWrapper(const RSAPrivateWrapper& rsaprivate);
	RSAPrivateWrapper& operator=(const RSAPrivateWrapper& rsaprivate);
public:
	RSAPrivateWrapper();
	RSAPrivateWrapper(const uint8_t* key, size_t length);
	RSAPrivateWrapper(const std::string& key);
	~RSAPrivateWrapper();

	std::string getPrivateKey() const;
	uint8_t* getPrivateKey(uint8_t* keyout, size_t length) const;

	std::string getPublicKey() const;
	uint8_t* getPublicKey(uint8_t* keyout, size_t length) const;

	std::string decrypt(const std::string& cipher);
	std::string decrypt(const uint8_t* cipher, size_t length);
};
