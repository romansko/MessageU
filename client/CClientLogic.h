#pragma once

#include "protocol.h"
#include <sstream>
#include <string>
#include <vector>

constexpr auto CLIENT_INFO = "me.info";   // Should be located near exe file.
constexpr auto SERVER_INFO = "server.info";  // Should be located near exe file.

class CFileHandler;
class CSocketHandler;
class RSAPrivateWrapper;

class CClientLogic
{
	struct SClient
	{
		SClientID     id;
		std::string   username;
		SPublicKey    publicKey;
		SSymmetricKey symmetricKey;
	};

public:
	CClientLogic();
	virtual ~CClientLogic();
	CClientLogic(const CClientLogic& other) = delete;
	CClientLogic(CClientLogic&& other) noexcept = delete;
	CClientLogic& operator=(const CClientLogic& other) = delete;
	CClientLogic& operator=(CClientLogic&& other) noexcept = delete;

	// static functions
	static std::string hex(const std::string& str);
	static std::string hex(const uint8_t* buffer, const size_t size);
	static std::string unhex(const std::string& str);
	static std::string unhex(const uint8_t* buffer, const size_t size);
	
	// inline getters
	std::string getLastError() const { return _lastError.str(); }
	std::string getSelfUsername()  const { return _self.username; }
	SClientID   getSelfClientID()  const { return _self.id; }
	
	// client logic to be invoked by client menu.
	bool parseServeInfo();
	bool parseClientInfo();
	bool getClientId(const std::string& username, SClientID& clientID) const;
	std::vector<std::string> getUsernames() const;
	bool registerClient(const std::string& username);
	bool requestClientsList();
	bool requestClientPublicKey(const std::string& username);
	bool sendMessage(const std::string& username, const EMessageType type, void* data = nullptr);

private:
	void clearLastError();
	bool storeClientInfo();
	bool validateHeader(const SResponseHeader& header, const EResponseCode expectedCode);
	bool receiveUnknownPayload(const EResponseCode expectedCode, uint8_t*& payload, size_t& size);
	bool setClientPublicKey(const SClientID& clientID, const SPublicKey& publicKey);
	bool getClientPublicKey(const SClientID& clientID, SPublicKey& publicKey);
	bool setClientSymmetricKey(const SClientID& clientID, const SSymmetricKey& symmetricKey);
	bool getClientSymmetricKey(const SClientID& clientID, SSymmetricKey& symmetricKey);

	SClient              _self;           // self symmetric key invalid.
	std::vector<SClient> _clients;
	std::stringstream    _lastError;
	CFileHandler*        _fileHandler;
	CSocketHandler*      _socketHandler;
	RSAPrivateWrapper*   _rsaDecryptor;
};
