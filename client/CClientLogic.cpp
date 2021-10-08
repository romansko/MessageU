#include "CClientLogic.h"

#include <iostream>
#include <iomanip>
#include <boost/algorithm/string/trim.hpp>
#include "Base64Wrapper.h"

CClientLogic::CClientLogic() : _rsaDecryptor(nullptr)
{
}

CClientLogic::~CClientLogic()
{
	delete _rsaDecryptor;
}


std::string CClientLogic::getLastError() const
{
	return _lastError.str();
}

std::string CClientLogic::hexify(const unsigned char* buffer, unsigned int length)
{
	std::stringstream hexified;
	const std::ios::fmtflags f(std::cout.flags());
	hexified << std::hex;
	for (size_t i = 0; i < length; i++)
		hexified << std::setfill('0') << std::setw(2) << (0xFF & buffer[i]) << (((i + 1) % 16 == 0) ? "\n" : " ");
	hexified << std::endl;
	hexified.flags(f);
	return hexified.str();
}

/**
 * Parse SERVER_INFO file for server address & port.
 */
bool CClientLogic::parseServeInfo()
{
	std::stringstream err;
	if (!_fileHandler.open(SERVER_INFO))
	{
		clearLastError();
		_lastError << "Couldn't open " << SERVER_INFO;
		return false;
	}
	std::string info;
	if (!_fileHandler.readLine(info))
	{
		clearLastError();
		_lastError << "Couldn't read " << SERVER_INFO;
		return false;
	}
	_fileHandler.close();
	boost::algorithm::trim(info);
	const auto pos = info.find(':');
	if (pos == std::string::npos)
	{
		clearLastError();
		_lastError << SERVER_INFO << " has invalid format! missing separator ':'";
		return false;
	}
	const auto address = info.substr(0, pos);
	const auto port = info.substr(pos + 1);
	if (!_socketHandler.setSocketInfo(address, port))
	{
		clearLastError();
		_lastError << SERVER_INFO << " has invalid IP address or port!";
		return false;
	}
	return true;
}

bool CClientLogic::parseClientInfo()
{
	std::string line;
	if (!_fileHandler.open(CLIENT_INFO))
	{
		clearLastError();
		_lastError << "Couldn't open " << CLIENT_INFO;
		return false;
	}

	// Read & Parse username
	if (!_fileHandler.readLine(line))
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
		_lastError << "Couldn't read client's UUID from " << CLIENT_INFO;
		return false;
	}
	line = Base64Wrapper::decode(line);
	if (line.size() > sizeof(_uuid.uuid))
	{
		clearLastError();
		_lastError << "Invalid UUID read from " << CLIENT_INFO;
		return false;
	}
	memcpy(_uuid.uuid, line.c_str(), sizeof(_uuid.uuid));

	// Read & Parse Client's private key.
	if (!_fileHandler.readLine(line))
	{
		clearLastError();
		_lastError << "Couldn't read client's private key from " << CLIENT_INFO;
		return false;
	}
	try
	{
		_rsaDecryptor = new RSAPrivateWrapper(Base64Wrapper::decode(line));
	}
	catch(...)
	{
		clearLastError();
		_lastError << "Couldn't parse private key from " << CLIENT_INFO;
		return false;
	}
	_fileHandler.close();
	return true;
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


bool CClientLogic::writeClientInfo()
{
	if (!_fileHandler.open(CLIENT_INFO))
	{
		clearLastError();
		_lastError << "Couldn't open " << CLIENT_INFO;
		return false;
	}

	// Write username
	if (!_fileHandler.write(reinterpret_cast<const uint8_t*>(_username.c_str()), _username.size()))
	{
		clearLastError();
		_lastError << "Couldn't write username to " << CLIENT_INFO;
		return false;
	}

	// Write UUID.
	if (!_fileHandler.write(_uuid.uuid, sizeof(_uuid.uuid)))
	{
		clearLastError();
		_lastError << "Couldn't write UUID to " << CLIENT_INFO;
		return false;
	}

	// Write Base64 encoded private key
	const auto encodedKey = Base64Wrapper::encode(_rsaDecryptor->getPrivateKey());
	if (!_fileHandler.write(reinterpret_cast<const uint8_t*>(encodedKey.c_str()), encodedKey.size()))
	{
		clearLastError();
		_lastError << "Couldn't write client's private key to " << CLIENT_INFO;
		return false;
	}

	_fileHandler.close();
	return true;
}

bool CClientLogic::validateHeader(const SResponseHeader& header, const EResponseCode expectedCode)
{
	// todo: version validation ?

	if (header.code == RESPONSE_ERROR)
	{
		clearLastError();
		_lastError << "Generic error response code (" << RESPONSE_ERROR << ") received.";
		return false;
	}
	
	if (header.code != expectedCode)
	{
		clearLastError();
		_lastError << "Unexpected response code " << header.code << " received. Expected code was " << expectedCode;
		return false;
	}

	switch (header.code)
	{
	case RESPONSE_REGISTRATION:
	{
		if (header.payloadSize != sizeof(SClientID))
		{
			clearLastError();
			_lastError << "Unexpected payload size " << header.payloadSize << ". Expected size was " << sizeof(SClientID);
			return false;
		}
		break;
	}
	default:
	{
		break;
	}
	}


	return true;
}



/**
 * Register client via the server.
 */
bool CClientLogic::registerClient(const std::string& username)
{
	SRegistrationRequest  request;
	SRegistrationResponse response;

	if (username.length() >= CLIENT_NAME_SIZE)  // >= because of null termination.
	{
		clearLastError();
		_lastError << "Invalid username length!";
		return false;
	}

	delete _rsaDecryptor;
	_rsaDecryptor = new RSAPrivateWrapper();
	const auto publicKey = _rsaDecryptor->getPublicKey();
	if (publicKey.size() != CLIENT_PUBLIC_KEY_SIZE)
	{
		clearLastError();
		_lastError << "Invalid public key length!";
		return false;
	}

	// fill request data
	request.header.code = REQUEST_REGISTRATION;
	request.header.payloadSize = sizeof(request.payload);
	memcpy(request.payload.name, username.c_str(), username.length());
	memcpy(request.payload.publicKey, publicKey.c_str(), sizeof(request.payload.publicKey));

	if (!_socketHandler.connect())
	{
		clearLastError();
		_lastError << "Failed connecting to server on " << _socketHandler;
		return false;
	}
	if (!_socketHandler.send(reinterpret_cast<const uint8_t* const>(&request), sizeof(request)))
	{
		clearLastError();
		_lastError << "Failed sending registration request to server on " << _socketHandler;
		return false;
	}
	if (!_socketHandler.receive(reinterpret_cast<uint8_t* const>(&response), sizeof(response)))
	{
		clearLastError();
		_lastError << "Failed receiving registration response from server on" << _socketHandler;
		return false;
	}
	_socketHandler.close();

	// parse and validate SRegistrationResponse
	if (!validateHeader(response.header, RESPONSE_REGISTRATION))
		return false;  // error message updated within.

	// store received client's ID
	_uuid = response.clientID;

	if (!writeClientInfo())
	{
		// todo: server registered but write to disk failed.	
	}

	return true;
}
