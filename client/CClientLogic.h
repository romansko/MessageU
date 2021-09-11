#pragma once

#include "CFileHandler.h"
#include "CSocketHandler.h"
#include "RSAWrapper.h"

constexpr auto CLIENT_INFO = "me.info";   // Should be located near exe file.

class CClientLogic
{
public:
	CClientLogic();
	std::string readUserInput() const;
	std::string getLastError() const;
	bool registerClient();

private:
	void clearLastError();
	bool readClientInfo();

	std::stringstream _lastError;
	CFileHandler      _fileHandler;
	CSocketHandler    _socketHandler;
	RSAPrivateWrapper _rsaPrivateWrapper;
	bool              _registered;
	std::string       _username;
	std::string       _uuid;
	std::string       _privateKey;

public:
	
};
