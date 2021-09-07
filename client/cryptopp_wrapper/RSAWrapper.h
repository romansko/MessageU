#pragma once

#include <osrng.h>
#include <rsa.h>

#include <string>



class RSAPublicWrapper
{
public:
	static const unsigned int KEYSIZE = 160;
	static const unsigned int BITS = 1024;

private:
	CryptoPP::AutoSeededRandomPool _rng;
	CryptoPP::RSA::PublicKey _publicKey;

	RSAPublicWrapper(const RSAPublicWrapper& rsapublic);
	RSAPublicWrapper& operator=(const RSAPublicWrapper& rsapublic);
public:

	RSAPublicWrapper(const char* key, unsigned int length);
	RSAPublicWrapper(const std::string& key);
	~RSAPublicWrapper();

	std::string getPublicKey() const;
	char* getPublicKey(char* keyout, unsigned int length) const;

	std::string encrypt(const std::string& plain);
	std::string encrypt(const char* plain, unsigned int length);
};


class RSAPrivateWrapper
{
public:
	static const unsigned int BITS = 1024;

private:
	CryptoPP::AutoSeededRandomPool _rng;
	CryptoPP::RSA::PrivateKey _privateKey;

	RSAPrivateWrapper(const RSAPrivateWrapper& rsaprivate);
	RSAPrivateWrapper& operator=(const RSAPrivateWrapper& rsaprivate);
public:
	RSAPrivateWrapper();
	RSAPrivateWrapper(const char* key, unsigned int length);
	RSAPrivateWrapper(const std::string& key);
	~RSAPrivateWrapper();

	std::string getPrivateKey() const;
	char* getPrivateKey(char* keyout, unsigned int length) const;

	std::string getPublicKey() const;
	char* getPublicKey(char* keyout, unsigned int length) const;

	std::string decrypt(const std::string& cipher);
	std::string decrypt(const char* cipher, unsigned int length);
};
