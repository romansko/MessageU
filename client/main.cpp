
#include "Base64Wrapper.h"
#include "RSAWrapper.h"
#include "AESWrapper.h"

#include <iostream>
#include <boost/algorithm/string/trim.hpp>  // for trimming strings

#include "CClientMenu.h"

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
	std::cout << CClientLogic::hex(ciphertext.c_str()) << std::endl;;	// print binary data nicely

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
	std::cout << CClientLogic::hex(plain, sizeof(plain)) << std::endl;		// print binary data nicely

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
	std::cout << CClientLogic::hex(cipher) << std::endl;	// print binary data nicely


	// 4. get the private key and encode it as base64 (base64 in not necessary for an RSA decryptor.)
	std::string base64key = Base64Wrapper::encode(rsapriv.getPrivateKey());

	// 5. create another RSA decryptor using an existing private key (decode the base64 key to an std::string first)
	RSAPrivateWrapper rsapriv_other(Base64Wrapper::decode(base64key));

	std::string decrypted = rsapriv_other.decrypt(cipher);		// 6. you can decrypt an std::string or a const char* buffer
	std::cout << "decrypted:" << std::endl;
	std::cout << CClientLogic::hex(decrypted) << std::endl;	// print binary data nicely

	return 0;
}


int main(int argc, char* argv[])
{
	//aes_example();
    //rsa_example();

	CClientMenu menu;
	menu.initialize();
	
	for (;;)
	{
		menu.display();
		menu.handleUserChoice();
		system("pause"); // todo remove
	}
	
	return 0;
}

