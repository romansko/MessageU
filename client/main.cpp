
#include "Base64Wrapper.h"
#include "RSAWrapper.h"
#include "AESWrapper.h"

#include <iostream>
#include <iomanip>
#include <boost/algorithm/string/trim.hpp>  // for trimming strings

#include "protocol.h"
#include "CClientMenu.h"
#include "CFileHandler.h"
#include "CSocketHandler.h"


// private globals
static CFileHandler   fileHandler;
static CSocketHandler socketHandler;
static constexpr auto SERVER_INFO = "server.info";  // Should be located near exe file.
static constexpr auto CLIENT_INFO = "me.info";      // Should be located near exe file.
static std::string address;   // server address
static std::string port;      // server port


void hexify(const unsigned char* buffer, unsigned int length)
{
	std::ios::fmtflags f(std::cout.flags());
	std::cout << std::hex;
	for (size_t i = 0; i < length; i++)
		std::cout << std::setfill('0') << std::setw(2) << (0xFF & buffer[i]) << (((i + 1) % 16 == 0) ? "\n" : " ");
	std::cout << std::endl;
	std::cout.flags(f);
}

int aes_example()
{
	std::cout << std::endl << std::endl << "----- AES EXAMPLE -----" << std::endl << std::endl;

	std::string plaintext = "Once upon a time, a plain text dreamed to become a cipher";
	std::cout << "Plain:" << std::endl << plaintext << std::endl;

	// 1. Generate a key and initialize an AESWrapper. You can also create AESWrapper with default constructor which will automatically generates a random key.
	unsigned char key[AESWrapper::DEFAULT_KEYLENGTH];
	AESWrapper aes(AESWrapper::GenerateKey(key, AESWrapper::DEFAULT_KEYLENGTH), AESWrapper::DEFAULT_KEYLENGTH);

	// 2. encrypt a message (plain text)
	std::string ciphertext = aes.encrypt(plaintext.c_str(), plaintext.length());
	std::cout << "Cipher:" << std::endl;
	hexify(reinterpret_cast<const unsigned char*>(ciphertext.c_str()), ciphertext.length());	// print binary data nicely

	// 3. decrypt a message (cipher text)
	std::string decrypttext = aes.decrypt(ciphertext.c_str(), ciphertext.length());
	std::cout << "Decrypted:" << std::endl << decrypttext << std::endl;

	return 0;
}

int rsa_example()
{
	std::cout << std::endl << std::endl << "----- RSA EXAMPLE -----" << std::endl << std::endl;

	// plain text (could be binary data as well)
	unsigned char plain[] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF };
	std::cout << "plain:" << std::endl;
	hexify(plain, sizeof(plain));		// print binary data nicely

	// 1. Create an RSA decryptor. this is done here to generate a new private/public key pair
	RSAPrivateWrapper rsapriv;

	// 2. get the public key
	std::string pubkey = rsapriv.getPublicKey();	// you can get it as std::string ...
	
	char pubkeybuff[RSAPublicWrapper::KEYSIZE]; 
	rsapriv.getPublicKey(pubkeybuff, RSAPublicWrapper::KEYSIZE);	// ...or as a char* buffer

	// 3. create an RSA encryptor
	RSAPublicWrapper rsapub(pubkey);
	std::string cipher = rsapub.encrypt((const char*)plain, sizeof(plain));	// you can encrypt a const char* or an std::string 
	std::cout << "cipher:" << std::endl;
	hexify((unsigned char*)cipher.c_str(), cipher.length());	// print binary data nicely


	// 4. get the private key and encode it as base64 (base64 in not necessary for an RSA decryptor.)
	std::string base64key = Base64Wrapper::encode(rsapriv.getPrivateKey());

	// 5. create another RSA decryptor using an existing private key (decode the base64 key to an std::string first)
	RSAPrivateWrapper rsapriv_other(Base64Wrapper::decode(base64key));

	std::string decrypted = rsapriv_other.decrypt(cipher);		// 6. you can decrypt an std::string or a const char* buffer
	std::cout << "decrypted:" << std::endl;
	hexify((unsigned char*)decrypted.c_str(), decrypted.length());	// print binary data nicely

	return 0;
}

static void clientStop(std::stringstream& err)
{
	std::cout << "Fatal Error: " << err.str() << std::endl << "Client will stop." << std::endl;
	exit(1);
}

/**
 * Parse SERVER_INFO file for server address & port.
 */
static void parseServeInfo()
{
	std::fstream fs;
	std::stringstream err;
	if (!fileHandler.fileOpen(SERVER_INFO, fs))
	{	
		err << "Couldn't open " << SERVER_INFO;
		clientStop(err);
	}
	std::string info;
	if (!fileHandler.fileReadLine(fs, info))
	{
		err << "Couldn't read " << SERVER_INFO;
		clientStop(err);
	}
	boost::algorithm::trim(info);
	const auto pos = info.find(':');
	if (pos == std::string::npos)
	{
		err << SERVER_INFO << " has invalid format! missing separator ':'";
		clientStop(err);
	}
	address = info.substr(0, pos);
	port = info.substr(pos + 1);
	if (!socketHandler.isValidIp(address) || !socketHandler.isValidPort(port))
	{
		err << SERVER_INFO << " has invalid IP Address or port format!";
		clientStop(err);
	}
}


int main(int argc, char* argv[])
{
	//aes_example();
	//rsa_example();

	parseServeInfo();
	std::cout << CSocketHandler::testSocket(address, port, "Test Socket..") << std::endl;
	system("pause");
	
	for (CClientMenu menu;;)
	{
		menu.display();
		menu.handleUserChoice();
		system("pause"); // todo remove
	}
	
	
	return 0;
}
