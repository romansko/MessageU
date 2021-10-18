#pragma once

#include <osrng.h>
#include <rsa.h>

#include <string>


struct SPublicKey;

class RSAPublicWrapper
{
public:
	static constexpr size_t KEYSIZE = 160;
	static constexpr size_t BITS    = 1024;

private:
	CryptoPP::AutoSeededRandomPool _rng;
	CryptoPP::RSA::PublicKey _publicKey;

public:
	RSAPublicWrapper(const SPublicKey& publicKey);
	RSAPublicWrapper(const uint8_t* key, size_t length);
	RSAPublicWrapper(const std::string& key);
	
	virtual ~RSAPublicWrapper() = default;
	RSAPublicWrapper(const RSAPublicWrapper& other) = delete;
	RSAPublicWrapper(RSAPublicWrapper&& other) noexcept = delete;
	RSAPublicWrapper& operator=(const RSAPublicWrapper& other) = delete;
	RSAPublicWrapper& operator=(RSAPublicWrapper&& other) noexcept = delete;

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


public:
	RSAPrivateWrapper();
	RSAPrivateWrapper(const uint8_t* key, size_t length);
	RSAPrivateWrapper(const std::string& key);

	virtual ~RSAPrivateWrapper() = default;
	RSAPrivateWrapper(const RSAPrivateWrapper& other) = delete;
	RSAPrivateWrapper(RSAPrivateWrapper&& other) noexcept = delete;
	RSAPrivateWrapper& operator=(const RSAPrivateWrapper& other) = delete;
	RSAPrivateWrapper& operator=(RSAPrivateWrapper&& other) noexcept = delete;
	
	std::string getPrivateKey() const;
	uint8_t* getPrivateKey(uint8_t* keyout, size_t length) const;

	std::string getPublicKey() const;
	uint8_t* getPublicKey(uint8_t* keyout, size_t length) const;

	std::string decrypt(const std::string& cipher);
	std::string decrypt(const uint8_t* cipher, size_t length);
};
