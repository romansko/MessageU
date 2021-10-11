/**
 * MessageU Client
 * @file protocol.h
 * @brief Define protocol between client & server according to the requirements.
 * @author Roman Koifman
 */

#pragma once
#include <cstdint>

#define DEF_VAL 0   // Default value used to initialize protocol structures.
constexpr uint8_t CLIENT_VERSION        = 2;
constexpr size_t CLIENT_ID_SIZE         = 16;
constexpr size_t CLIENT_NAME_SIZE       = 255;
constexpr size_t CLIENT_PUBLIC_KEY_SIZE = 160;
constexpr size_t REQUEST_OPTIONS        = 5;
constexpr size_t RESPONSE_OPTIONS       = 6;

enum ERequestCode
{
	REQUEST_REGISTRATION   = 1000,   // uuid ignored.
	REQUEST_USERS          = 1001,   // payload invalid. payloadSize = 0.
	REQUEST_PUBLIC_KEY     = 1002,
	REQUEST_SEND_MSG       = 1003,
	REQUEST_PENDING_MSG    = 1004    // payload invalid. payloadSize = 0.
};

enum EResponseCode
{
	RESPONSE_REGISTRATION  = 2000,
	RESPONSE_USERS         = 2001,
	RESPONSE_PUBLIC_KEY    = 2002,
	RESPONSE_MSG_SENT      = 2003,
	RESPONSE_PENDING_MSG   = 2004,
	RESPONSE_ERROR         = 9000    // payload invalid. payloadSize = 0.
};

enum EMessageType
{
	MSG_INVALID = DEF_VAL,
	MSG_SYMMETRIC_KEY_REQUEST,   // content invalid. contentSize = 0.
	MSG_SYMMETRIC_KEY,           // content = symmetric key encrypted by destination client's public key.
	MSG_ENCRYPTED,                // content = encrypted message by symmetric key.
	MSG_FILE                    // content = encrypted file by symmetric key.
};

#pragma pack(push, 1)

struct SClientID
{
	uint8_t uuid[CLIENT_ID_SIZE];
	SClientID(): uuid{DEF_VAL} {}

	bool operator==(SClientID& otherID)
	{
		for (size_t i = 0; i < CLIENT_ID_SIZE; ++i)
			if (uuid[i] != otherID.uuid[i])
				return false;
		return true;
	}
	bool operator!=(SClientID& otherID)
	{
		return !(*this == otherID);
	}
};

struct SClientName
{
	uint8_t name[CLIENT_NAME_SIZE];  // DEF_VAL terminated.
	SClientName(): name{ '\0' } {}
};

struct SPublicKey
{
	uint8_t publicKey[CLIENT_PUBLIC_KEY_SIZE];
	SPublicKey(): publicKey{ DEF_VAL } {}
};

struct SClientIDName
{
	SClientID   clientId;
	SClientName clientName;
};

struct SClientIDPublicKey
{
	SClientID   clientId;
	SPublicKey  clientPublicKey;
};

struct SClientNamePublicKey
{
	SClientName clientName;
	SPublicKey  clientPublicKey;
};

struct SRequestHeader
{
	SClientID     clientID;
    const uint8_t version;
    uint16_t      code;
	uint32_t      payloadSize;
    SRequestHeader(): version(CLIENT_VERSION), code(DEF_VAL), payloadSize(DEF_VAL) {}
};

struct SResponseHeader
{
	uint8_t   version;
	uint16_t  code;
	uint32_t  payloadSize;
	SResponseHeader(): version(DEF_VAL), code(DEF_VAL), payloadSize(DEF_VAL) {}
};

struct SRequestRegistration
{
	SRequestHeader       header;
	SClientNamePublicKey payload;
};

struct SResponseRegistration
{
	SResponseHeader header;
	SClientID       payload;
};

struct SRequestPublicKey
{
	SRequestHeader header;
	SClientID      payload;
};

struct SResponsePublicKey
{
	SResponseHeader    header;
	SClientIDPublicKey payload;
};

struct SMessageSent
{
	SClientID clientId;   // destination
	uint32_t  messageId;
	SMessageSent(): messageId(DEF_VAL) {}
};

struct SMessagePending
{
	SClientID clientId;   // source
	uint32_t  messageId;
	uint8_t   messageType;
	uint32_t  messageSize;
	SMessagePending(): messageId(DEF_VAL), messageType(DEF_VAL), messageSize(DEF_VAL) {}
};

#pragma pack(pop)

