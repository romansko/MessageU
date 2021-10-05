#include "CClientLogic.h"
#include "protocol.h"
#include <iostream>
#include <boost/algorithm/string/trim.hpp>

CClientLogic::CClientLogic() : _registered(false)
{
	
}

/**
 * Read input from console.
 */
std::string CClientLogic::readUserInput() const
{
	std::string input;
	do
	{
		std::getline(std::cin, input);
		boost::algorithm::trim(input);
	} while (input.empty());
	
	return input;
}


std::string CClientLogic::getLastError() const
{
	return _lastError.str();
}


/**
 * Reset _lastError StringStream: Empty string, clear errors flag and reset formatting.
 */
void CClientLogic::clearLastError()
{
	const std::stringstream clean;
	_lastError.str("");
	_lastError.clear();
	_lastError.copyfmt(clean);
}

bool CClientLogic::readClientInfo()
{
	std::fstream fs;
	std::string line;
	if (!_fileHandler.open(CLIENT_INFO))
	{
		clearLastError();
		_lastError << "Couldn't open " << CLIENT_INFO;
		return false;
	}

	// Read & Parse username
	if(!_fileHandler.readLine(line))
	{
		clearLastError();
		_lastError << "Couldn't read username from " << CLIENT_INFO;
		return false;
	}
	boost::algorithm::trim(line);
	if (line.length() >= CLIENT_NAME_SIZE)
	{
		clearLastError();
		_lastError << "Invalid username read from " << CLIENT_INFO;
		return false;
	}
	_username = line;

	// Read & Parse Client's UUID.
	if (!_fileHandler.readLine(line))
	{
		clearLastError();
		_lastError << "Couldn't read client's uuid from " << CLIENT_INFO;
		return false;
	}
	_uuid = line;

	// Read & Parse Client's private key.
	if (!_fileHandler.readLine(line))
	{
		clearLastError();
		_lastError << "Couldn't read client's private key from " << CLIENT_INFO;
		return false;
	}
	_privateKey = line;
	_registered = true;
	_fileHandler.close();
	return true;
}


/**
 * Register client via the server.
 */
bool CClientLogic::registerClient()
{
	if (_registered)
	{
		clearLastError();
		_lastError << "Client already registered!";
		return false;
	}

	std::cout << "Please type your username.." << std::endl;
	auto username = readUserInput();  // name should be less than CLIENT_NAME_SIZE (null terminated).
	while (username.length() >= CLIENT_NAME_SIZE)
	{
		std::cout << "Invalid username (Empty or too long username). Please try again.." << std::endl;
		username = readUserInput();
	} 
	SRequestHeader       regHeader;
	SPayloadRegistration regPayload;
	regHeader.code = REQUEST_REGISTRATION;
	regHeader.payloadSize = sizeof(SPayloadRegistration);
	memcpy(regPayload.name, username.c_str(), username.length());

	const auto privateKey = _rsaPrivateWrapper.getPrivateKey();
	const auto publicKey  = _rsaPrivateWrapper.getPublicKey();
	
	if (publicKey.size() != CLIENT_PUBLIC_KEY_SIZE)
	{
		clearLastError();
		_lastError << "Invalid public key length!";
		return false;
	}
	memcpy(regPayload.publicKey, publicKey.c_str(), publicKey.length());

	// todo: Send to server. Receive UUID. Save to me.info..
	

	_registered = true;
	return true;
}
