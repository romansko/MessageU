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

std::string CClientLogic::hexify(const uint8_t* buffer, const size_t size)
{
	std::stringstream hexified;
	const std::ios::fmtflags f(std::cout.flags());
	hexified << std::hex;
	for (size_t i = 0; i < size; ++i)
		hexified << std::setfill('0') << std::setw(2) << (0xFF & buffer[i]);
	hexified.flags(f);
	return hexified.str();
}

bool CClientLogic::unhexify(const std::string& hexString, uint8_t* const buffer, const size_t size)
{
	if ((hexString.length() / 2) != size)  // Each byte is represented by two ASCII chars.
		return false;
	try
	{
		for (size_t i = 0; i < size; ++i)
		{
			buffer[i] = static_cast<uint8_t>(std::stoi(hexString.substr(i * 2, 2), nullptr, 16));
		}
	}
	catch (...)
	{
		return false;
	}
	return true;
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
	if (!unhexify(line, _clientID.uuid, sizeof(_clientID.uuid)))
	{
		memset(_clientID.uuid, 0, sizeof(_clientID.uuid));
		clearLastError();
		_lastError << "Couldn't parse client's UUID from " << CLIENT_INFO;
		return false;
	}

	// Read & Parse Client's private key.
	std::string decodedKey;
	while (_fileHandler.readLine(line))
	{
		decodedKey.append(Base64Wrapper::decode(line));
	}
	if (decodedKey.empty())
	{
		clearLastError();
		_lastError << "Couldn't read client's private key from " << CLIENT_INFO;
		return false;
	}
	try
	{
		_rsaDecryptor = new RSAPrivateWrapper(decodedKey);
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

std::string CClientLogic::getUserID(const std::string& username) const
{
	if (username == _username)
		return hexify(_clientID.uuid, sizeof(_clientID.uuid));
	const auto it = std::find_if(_usersList.begin(), _usersList.end(),
		[&username](const std::pair<std::string, std::string>& client) {
			return (client.second == username);
		});
	return (it == _usersList.end()) ? "" : it->first;
}

/**
 * Copy usernames into vector & sort them alphabetically.
 * If _usersList is empty, an empty vector will be returned.
 */
std::vector<std::string> CClientLogic::getUsernames() const
{
	std::vector<std::string> usernames(_usersList.size());
	std::transform(_usersList.begin(), _usersList.end(), usernames.begin(),
		[](const std::pair<std::string, std::string>& client) { return client.second; });
	std::sort(usernames.begin(), usernames.end());
	return usernames;
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


bool CClientLogic::storeClientInfo()
{
	if (!_fileHandler.open(CLIENT_INFO, true))
	{
		clearLastError();
		_lastError << "Couldn't open " << CLIENT_INFO;
		return false;
	}

	// Write username
	if (!_fileHandler.writeLine(_username))
	{
		clearLastError();
		_lastError << "Couldn't write username to " << CLIENT_INFO;
		return false;
	}

	// Write UUID.
	const auto hexifiedUUID = hexify(_clientID.uuid, sizeof(_clientID.uuid));
	if (!_fileHandler.writeLine(hexifiedUUID))
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
		if (header.payloadSize != (sizeof(SResponseRegistration) - sizeof(SResponseHeader)))
		{
			clearLastError();
			_lastError << "Unexpected payload size " << header.payloadSize << ". Expected size was " << sizeof(SClientID);
			return false;
		}
		break;
	}
	case RESPONSE_PUBLIC_KEY:
	{
		if (header.payloadSize != (sizeof(SResponsePublicKey) - sizeof(SResponseHeader)))
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

bool CClientLogic::receiveUnknownPayload(const EResponseCode expectedCode, uint8_t*& payload, size_t& size)
{
	SResponseHeader response;
	uint8_t buffer[PACKET_SIZE];
	payload = nullptr;
	size = 0;
	if (!_socketHandler.receive(buffer, sizeof(buffer)))
	{
		clearLastError();
		_lastError << "Failed receiving response header from server on " << _socketHandler;
		return false;
	}
	memcpy(&response, buffer, sizeof(SResponseHeader));
	if (!validateHeader(response, expectedCode))
	{
		clearLastError();
		_lastError << "Received unexpected response code from server on  " << _socketHandler;
		return false;
	}
	if (response.payloadSize == 0)
		return true;  // no payload. but not an error.

	size = response.payloadSize;
	payload = new uint8_t[size];
	uint8_t* ptr = static_cast<uint8_t*>(buffer) + sizeof(SResponseHeader);
	size_t recSize = sizeof(buffer) - sizeof(SResponseHeader);
	if (recSize > size)
		recSize = size;
	memcpy(payload, ptr, recSize);
	ptr = payload + recSize;
	while(recSize < size)
	{
		size_t toRead = (size - recSize);
		if (toRead > PACKET_SIZE)
			toRead = PACKET_SIZE;
		if (!_socketHandler.receive(buffer, toRead))
		{
			clearLastError();
			_lastError << "Failed receiving payload data from server on " << _socketHandler;
			delete[] payload;
			payload = nullptr;
			size = 0;
			return false;
		}
		memcpy(ptr, buffer, toRead);
		recSize += toRead;
		ptr += toRead;
	}
	
	return true;
}


/**
 * Register client via the server.
 */
bool CClientLogic::registerClient(const std::string& username)
{
	SRequestRegistration  request;
	SResponseRegistration response;

	if (username.length() >= CLIENT_NAME_SIZE)  // >= because of null termination.
	{
		clearLastError();
		_lastError << "Invalid username length!";
		return false;
	}
	for (auto ch : username)
	{
		if (!std::isalnum(ch))  // check that username is alphanumeric. [a-zA-Z0-9].
		{
			clearLastError();
			_lastError << "Invalid username! Username may only contain letters and numbers!";
			return false;
		}
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
	memcpy(request.payload.clientName.name, username.c_str(), username.length());
	memcpy(request.payload.clientPublicKey.publicKey, publicKey.c_str(), sizeof(request.payload.clientPublicKey.publicKey));

	if (!_socketHandler.sendReceive(reinterpret_cast<const uint8_t* const>(&request), sizeof(request),
		reinterpret_cast<uint8_t* const>(&response), sizeof(response)))
	{
		clearLastError();
		_lastError << "Failed communicating with server on " << _socketHandler;
		return false;
	}

	// parse and validate SResponseRegistration
	if (!validateHeader(response.header, RESPONSE_REGISTRATION))
		return false;  // error message updated within.

	// store received client's ID
	_clientID = response.payload;
	_username = username;
	if (!storeClientInfo())
	{
		// todo: server registered but write to disk failed
		clearLastError();
		_lastError << "Failed writing client info to " << CLIENT_INFO;
		return false;
	}

	return true;
}

bool CClientLogic::requestClientsList()
{
	SRequestHeader request;
	SResponseHeader response;
	uint8_t* payload = nullptr;
	size_t payloadSize = 0;
	
	request.code = REQUEST_USERS;
	request.clientID = _clientID;
	if (!_socketHandler.connect())
	{
		clearLastError();
		_lastError << "Failed connecting to server on " << _socketHandler;
		return false;
	}
	if (!_socketHandler.send(reinterpret_cast<const uint8_t*>(&request), sizeof(request)))
	{
		_socketHandler.close();
		clearLastError();
		_lastError << "Failed sending clients list request to server on " << _socketHandler;
		return false;
	}
	if (!receiveUnknownPayload(RESPONSE_USERS,payload, payloadSize))
	{
		_socketHandler.close();
		_lastError << " (Clients list request).";
		return false;
	}
	if (payloadSize == 0)
	{
		_socketHandler.close();
		clearLastError();
		_lastError << "Server has no users registered. Empty Clients list.";
		return false;
	}
	if (payloadSize % sizeof(SClientIDName) != 0)
	{
		_socketHandler.close();
		clearLastError();
		_lastError << "Clients list received is corrupted! (Invalid size).";
		delete[] payload;
		return false;
	}
	_socketHandler.close();
	uint8_t* ptr = payload;
	size_t parsedBytes = 0;
	SClientIDName client;
	_usersList.clear();
	while (parsedBytes < payloadSize)
	{
		memcpy(&client, ptr, sizeof(SClientIDName));
		ptr += sizeof(SClientIDName);
		parsedBytes += sizeof(SClientIDName);
		client.clientName.name[sizeof(client.clientName.name) - 1] = '\0'; // just in case..
		const std::string name = reinterpret_cast<char*>(client.clientName.name);
		const std::string clientID = hexify(client.clientId.uuid, sizeof(client.clientId.uuid));
		_usersList.insert(std::pair<std::string, std::string>(clientID, name));
	}
	delete[] payload;
	return true;
}

bool CClientLogic::requestClientPublicKey(const std::string& username, std::string& publicKey)
{
	publicKey = "";
	const std::string userID = getUserID(username);
	if (userID.empty())
	{
		clearLastError();
		_lastError << "username '" << username << "' doesn't exist. Please check your input or try to request users list again.";
		return false;
	}

	SRequestPublicKey  request;
	SResponsePublicKey response;
	request.header.code = REQUEST_PUBLIC_KEY;
	request.header.clientID = _clientID;
	if (!unhexify(userID, request.payload.uuid, sizeof(request.payload.uuid)))
	{
		clearLastError();
		_lastError << "Invalid userID: " << userID;
		return false;
	}

	if (!_socketHandler.sendReceive(reinterpret_cast<const uint8_t* const>(&request), sizeof(request),
		reinterpret_cast<uint8_t* const>(&response), sizeof(response)))
	{
		clearLastError();
		_lastError << "Failed communicating with server on " << _socketHandler;
		return false;
	}

	// parse and validate SResponseRegistration
	if (!validateHeader(response.header, RESPONSE_PUBLIC_KEY))
		return false;  // error message updated within.

	if (request.payload != response.payload.clientId)
	{
		clearLastError();
		_lastError << "Unexpected clientID was received.";
		return false;
	}

	publicKey = hexify(response.payload.clientPublicKey.publicKey, sizeof(response.payload.clientPublicKey.publicKey));
	
	return true;
}
