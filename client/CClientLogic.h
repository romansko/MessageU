#pragma once

#include "protocol.h"
#include "CFileHandler.h"
#include "CSocketHandler.h"
#include "RSAWrapper.h"

constexpr auto CLIENT_INFO = "me.info";   // Should be located near exe file.
constexpr auto SERVER_INFO = "server.info";  // Should be located near exe file.

class CClientLogic
{
public:
	CClientLogic();
	~CClientLogic();
	CClientLogic(const CClientLogic& other) = delete;
	CClientLogic(CClientLogic&& other) noexcept = delete;
	CClientLogic& operator=(const CClientLogic& other) = delete;
	CClientLogic& operator=(CClientLogic&& other) noexcept = delete;
	
	std::string getLastError() const;
	static std::string hexify(const uint8_t* buffer, size_t length);
	bool parseServeInfo();
	bool parseClientInfo();
	bool registerClient(const std::string& username);
	bool requestClientsList(bool registered);

private:
	void clearLastError();
	bool storeClientInfo();
	bool validateHeader(const SResponseHeader& header, const EResponseCode expectedCode);

	std::stringstream  _lastError;
	CFileHandler       _fileHandler;
	CSocketHandler     _socketHandler;
	RSAPrivateWrapper* _rsaDecryptor;
	std::string        _username;
	SClientID          _uuid;

public:
	
};
