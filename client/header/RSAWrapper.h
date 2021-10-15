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

	RSAPublicWrapper(const char* key, size_t length);
	RSAPublicWrapper(const std::string& key);
	~RSAPublicWrapper();

	std::string getPublicKey() const;
	char* getPublicKey(char* keyout, size_t length) const;

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
	RSAPrivateWrapper(const char* key, size_t length);
	RSAPrivateWrapper(const std::string& key);
	~RSAPrivateWrapper();

	std::string getPrivateKey() const;
	char* getPrivateKey(char* keyout, size_t length) const;

	std::string getPublicKey() const;
	char* getPublicKey(char* keyout, size_t length) const;

	std::string decrypt(const std::string& cipher);
	std::string decrypt(const char* cipher, size_t length);
};
