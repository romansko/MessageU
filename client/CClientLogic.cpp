#include "CClientLogic.h"

#include <iostream>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/hex.hpp>

#include "Base64Wrapper.h"
#include "RSAWrapper.h"
#include "AESWrapper.h"
#include "CFileHandler.h"
#include "CSocketHandler.h"

CClientLogic::CClientLogic() : _fileHandler(nullptr), _socketHandler(nullptr), _rsaDecryptor(nullptr)
{
	// Validate protocol's key size vs. Crypto utilities.
	assert(AESWrapper::DEFAULT_KEYLENGTH == SYMMETRIC_KEY_SIZE);
	assert(RSAPublicWrapper::KEYSIZE == PUBLIC_KEY_SIZE);
	_fileHandler   = new CFileHandler();
	_socketHandler = new CSocketHandler();
}

CClientLogic::~CClientLogic()
{
	delete _fileHandler;
	delete _socketHandler;
	delete _rsaDecryptor;
}

std::ostream& operator<<(std::ostream& os, const EMessageType& type)
{
	os << static_cast<messageType_t>(type);
	return os;
}

std::string CClientLogic::hex(const std::string& str)
{
	if (str.empty())
		return "";
	try
	{
		return boost::algorithm::hex(str);
	}
	catch(...)
	{
		return "";
	}
}

std::string CClientLogic::hex(const uint8_t* buffer, const size_t size)
{
	if (size == 0 || buffer == nullptr)
		return "";
	return hex(std::string(buffer, buffer + size));
}

std::string CClientLogic::unhex(const std::string& str)
{
	if (str.empty())
		return "";
	try
	{
		return boost::algorithm::unhex(str);
	}
	catch (...)
	{
		return "";
	}
}

std::string CClientLogic::unhex(const uint8_t* buffer, const size_t size)
{
	if (size == 0 || buffer == nullptr)
		return "";
	return unhex(std::string(buffer, buffer + size));
}

/**
 * Parse SERVER_INFO file for server address & port.
 */
bool CClientLogic::parseServeInfo()
{
	std::stringstream err;
	if (!_fileHandler->open(SERVER_INFO))
	{
		clearLastError();
		_lastError << "Couldn't open " << SERVER_INFO;
		return false;
	}
	std::string info;
	if (!_fileHandler->readLine(info))
	{
		clearLastError();
		_lastError << "Couldn't read " << SERVER_INFO;
		return false;
	}
	_fileHandler->close();
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
	if (!_socketHandler->setSocketInfo(address, port))
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
	if (!_fileHandler->open(CLIENT_INFO))
	{
		clearLastError();
		_lastError << "Couldn't open " << CLIENT_INFO;
		return false;
	}

	// Read & Parse username
	if (!_fileHandler->readLine(line))
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
	_self.username = line;

	// Read & Parse Client's UUID.
	if (!_fileHandler->readLine(line))
	{
		clearLastError();
		_lastError << "Couldn't read client's UUID from " << CLIENT_INFO;
		return false;
	}

	line = unhex(line);
	const char* unhexed = line.c_str();
	if (strlen(unhexed) != sizeof(_self.id.uuid))
	{
		memset(_self.id.uuid, 0, sizeof(_self.id.uuid));
		clearLastError();
		_lastError << "Couldn't parse client's UUID from " << CLIENT_INFO;
		return false;
	}
	memcpy(_self.id.uuid, unhexed, sizeof(_self.id.uuid));

	// Read & Parse Client's private key.
	std::string decodedKey;
	while (_fileHandler->readLine(line))
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
	_fileHandler->close();
	return true;
}

/**
 * Given a username, get a client ID from clients list.
 */
bool CClientLogic::getClientId(const std::string& username, SClientID& clientID) const
{
	if (username.empty())
		return false;
	
	if (username == _self.username)
	{
		clientID = _self.id;  // return self ID.
		return true;
	}
	
	for (const SClient& client : _clients)
	{
		if (username == client.username)
		{
			clientID = client.id;
			return true;
		}
	}
	
	return false; // clientID invalid.
}

/**
 * Copy usernames into vector & sort them alphabetically.
 * If _clients is empty, an empty vector will be returned.
 */
std::vector<std::string> CClientLogic::getUsernames() const
{
	std::vector<std::string> usernames(_clients.size());
	std::transform(_clients.begin(), _clients.end(), usernames.begin(),
		[](const SClient& client) { return client.username; });
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
	if (!_fileHandler->open(CLIENT_INFO, true))
	{
		clearLastError();
		_lastError << "Couldn't open " << CLIENT_INFO;
		return false;
	}

	// Write username
	if (!_fileHandler->writeLine(_self.username))
	{
		clearLastError();
		_lastError << "Couldn't write username to " << CLIENT_INFO;
		return false;
	}

	// Write UUID.
	const auto hexifiedUUID = hex(_self.id.uuid, sizeof(_self.id.uuid));
	if (!_fileHandler->writeLine(hexifiedUUID))
	{
		clearLastError();
		_lastError << "Couldn't write UUID to " << CLIENT_INFO;
		return false;
	}

	// Write Base64 encoded private key
	const auto encodedKey = Base64Wrapper::encode(_rsaDecryptor->getPrivateKey());
	if (!_fileHandler->write(reinterpret_cast<const uint8_t*>(encodedKey.c_str()), encodedKey.size()))
	{
		clearLastError();
		_lastError << "Couldn't write client's private key to " << CLIENT_INFO;
		return false;
	}

	_fileHandler->close();
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

	csize_t expectedSize = DEF_VAL;
	switch (header.code)
	{
	case RESPONSE_REGISTRATION:
	{
		expectedSize = sizeof(SResponseRegistration) - sizeof(SResponseHeader);
		break;
	}
	case RESPONSE_PUBLIC_KEY:
	{
		expectedSize = sizeof(SResponsePublicKey) - sizeof(SResponseHeader);
		break;
	}
	case RESPONSE_MSG_SENT:
	{
		expectedSize = sizeof(SResponseMessageSent) - sizeof(SResponseHeader);
		break;
	}
	default:
	{
		return true;  // variable payload size. 
	}
	}

	if (header.payloadSize != expectedSize)
	{
		clearLastError();
		_lastError << "Unexpected payload size " << header.payloadSize << ". Expected size was " << expectedSize;
		return false;
	}
	
	return true;
}

bool CClientLogic::receiveUnknownPayload(const EResponseCode expectedCode, uint8_t*& payload, size_t& size)
{
	SResponseHeader response;
	uint8_t buffer[PACKET_SIZE];
	payload = nullptr;
	size = 0;
	if (!_socketHandler->receive(buffer, sizeof(buffer)))
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
		if (!_socketHandler->receive(buffer, toRead))
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

bool CClientLogic::setClientPublicKey(const SClientID& clientID, const SPublicKey& publicKey)
{
	for (SClient& client : _clients)
	{
		if (client.id == clientID)
		{
			client.publicKey = publicKey;
			return true;
		}
	}
	clearLastError();
	_lastError << "Failed storing public key of a given clientID. Please try to request clients list again..";
	return false;
}

bool CClientLogic::getClientPublicKey(const SClientID& clientID, SPublicKey& publicKey)
{
	for (SClient& client : _clients)
	{
		if (client.id == clientID)
		{
			publicKey = client.publicKey;
			return true;
		}
	}
	clearLastError();
	_lastError << "Failed retrieving public key of a given clientID. Please try to request clients list again..";
	return false;  // publicKey invalid.
}

bool CClientLogic::setClientSymmetricKey(const SClientID& clientID, const SSymmetricKey& symmetricKey)
{
	for (SClient& client : _clients)
	{
		if (client.id == clientID)
		{
			client.symmetricKey = symmetricKey;
			return true;
		}
	}
	clearLastError();
	_lastError << "Failed storing symmetric key of a given clientID. Please try to request clients list again..";
	return false;
}

bool CClientLogic::getClientSymmetricKey(const SClientID& clientID, SSymmetricKey& symmetricKey)
{
	for (SClient& client : _clients)
	{
		if (client.id == clientID)
		{
			symmetricKey = client.symmetricKey;
			return true;
		}
	}
	clearLastError();
	_lastError << "Failed retrieving symmetric key of a given clientID. Please try to request clients list again..";
	return false;  // publicKey invalid.
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
	if (publicKey.size() != PUBLIC_KEY_SIZE)
	{
		clearLastError();
		_lastError << "Invalid public key length!";
		return false;
	}

	// fill request data
	request.header.payloadSize = sizeof(request.payload);
	strcpy_s(reinterpret_cast<char*>(request.payload.clientName.name), username.size(), username.c_str());
	memcpy(request.payload.clientPublicKey.publicKey, publicKey.c_str(), sizeof(request.payload.clientPublicKey.publicKey));

	if (!_socketHandler->sendReceive(reinterpret_cast<const uint8_t* const>(&request), sizeof(request),
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
	_self.id        = response.payload;
	_self.username  = username;
	_self.publicKey = request.payload.clientPublicKey;
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
	SRequestClientsList request;
	uint8_t* payload   = nullptr;
	uint8_t* ptr       = nullptr;
	size_t payloadSize = 0;
	size_t parsedBytes = 0;
	struct
	{
		SClientID   clientId;
		SClientName clientName;
	}client;
	
	request.header.clientId = _self.id;
	if (!_socketHandler->connect())
	{
		clearLastError();
		_lastError << "Failed connecting to server on " << _socketHandler;
		return false;
	}
	if (!_socketHandler->send(reinterpret_cast<const uint8_t*>(&request), sizeof(request)))
	{
		_socketHandler->close();
		clearLastError();
		_lastError << "Failed sending clients list request to server on " << _socketHandler;
		return false;
	}
	if (!receiveUnknownPayload(RESPONSE_USERS,payload, payloadSize))
	{
		_socketHandler->close();
		_lastError << " (Clients list request).";
		return false;
	}
	if (payloadSize == 0)
	{
		_socketHandler->close();
		clearLastError();
		_lastError << "Server has no users registered. Empty Clients list.";
		return false;
	}
	if (payloadSize % sizeof(client) != 0)
	{
		_socketHandler->close();
		clearLastError();
		_lastError << "Clients list received is corrupted! (Invalid size).";
		delete[] payload;
		return false;
	}
	_socketHandler->close();
	ptr = payload;
	_clients.clear();
	while (parsedBytes < payloadSize)
	{
		memcpy(&client, ptr, sizeof(client));
		ptr += sizeof(client);
		parsedBytes += sizeof(client);
		client.clientName.name[sizeof(client.clientName.name) - 1] = '\0'; // just in case..
		_clients.push_back({ client.clientId, reinterpret_cast<char*>(client.clientName.name) });
	}
	delete[] payload;
	return true;
}

bool CClientLogic::requestClientPublicKey(const std::string& username)
{
	SRequestPublicKey  request;
	SResponsePublicKey response;
	request.header.clientId = _self.id;
	
	if (!getClientId(username, request.payload))
	{
		clearLastError();
		_lastError << "username '" << username << "' doesn't exist. Please check your input or try to request users list again.";
		return false;
	}

	if (!_socketHandler->sendReceive(reinterpret_cast<const uint8_t* const>(&request), sizeof(request),
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

	// Set public key.
	if (!setClientPublicKey(response.payload.clientId, response.payload.clientPublicKey))
	{
		clearLastError();
		_lastError << "Couldn't assign public key for user " << username << ". ClientID was not found. Please try retrieve users list again..";
		return false;
	}
	return true;
}


bool CClientLogic::sendMessage(const std::string& username, const EMessageType type, void* data)
{
	SRequestSendMessage  request(_self.id, static_cast<messageType_t>(type));
	SResponseMessageSent response;
	SSymmetricKey symKey;
	uint8_t* content = nullptr, *msgToSend = nullptr;
	size_t msgSize = 0;

	if (!getClientId(username, request.payloadHeader.clientId))
	{
		clearLastError();
		_lastError << "username '" << username << "' doesn't exist. Please check your input or try to request users list again.";
		return false;
	}

	switch (type)  // Handle payload
	{
	case EMessageType::MSG_SYMMETRIC_KEY_REQUEST:
	{
		/* Do nothing */
		break;
	}
	case EMessageType::MSG_SYMMETRIC_KEY_SEND:
	{
		SPublicKey publicKey;
		AESWrapper aes(AESWrapper::GenerateKey(symKey.symmetricKey, SYMMETRIC_KEY_SIZE), SYMMETRIC_KEY_SIZE);
		if (!setClientSymmetricKey(request.payloadHeader.clientId, symKey))
			return false;  // error described within.
		if (!getClientPublicKey(request.payloadHeader.clientId, publicKey))
			return false;  // error described within.
		RSAPublicWrapper rsa(_rsaDecryptor->getPublicKey());
		const std::string encryptedKey = rsa.encrypt(symKey.symmetricKey, sizeof(symKey.symmetricKey));
		content = new uint8_t[encryptedKey.size()];
		memcpy(content, encryptedKey.c_str(), encryptedKey.size());
		request.payloadHeader.contentSize = encryptedKey.size();	
		break;
	}
	case EMessageType::MSG_TEXT:
	{
		std::cout << "unimp MSG_TEXT" << std::endl;
		break;
	}
	case EMessageType::MSG_FILE:
	{
		std::cout << "unimp MSG_FILE" << std::endl;
		break;
	}
	case EMessageType::MSG_INVALID:
	default:
	{
		clearLastError();
		_lastError << "Invalid message type: " << type;
		return false;
	}
	}

	// prepare message to send
	request.header.payloadSize = sizeof(request.payloadHeader) + request.payloadHeader.contentSize;
	if (content == nullptr)
	{
		msgToSend = reinterpret_cast<uint8_t*>(&request);
		msgSize   = sizeof(request);
	}
	else
	{
		msgToSend = new uint8_t[sizeof(request) + request.payloadHeader.contentSize];
		memcpy(msgToSend, &request, sizeof(request));
		memcpy(msgToSend + sizeof(request), content, request.payloadHeader.contentSize);
		msgSize = sizeof(request) + request.payloadHeader.contentSize;
	}

	// send request and receive response
	if (!_socketHandler->sendReceive(msgToSend, msgSize, reinterpret_cast<uint8_t* const>(&response), sizeof(response)))
	{
		delete[] content;
		if (msgToSend != reinterpret_cast<uint8_t*>(&request))
			delete[] msgToSend;
		clearLastError();
		_lastError << "Failed communicating with server on " << _socketHandler;
		return false;
	}

	delete[] content;
	if (msgToSend != reinterpret_cast<uint8_t*>(&request))
		delete[] msgToSend;

	// Validate SResponseMessageSent header
	if (!validateHeader(response.header, RESPONSE_MSG_SENT))
		return false;  // error message updated within.

	// Validate destination clientID
	if (request.payloadHeader.clientId != response.payload.clientId)
	{
		clearLastError();
		_lastError << "Unexpected clientID was received.";
		return false;
	}

	std::cout << "Message ID: " << response.payload.messageId << std::endl; // todo debug remove.

	return true;
}

