"""
MessageU Server
protocol.py: defines the protocol.
"""
__author__ = "Roman Koifman"

import struct
from enum import Enum

DEF_VAL = 0
HEADER_SIZE = 7  # Header size without clientID
CLIENT_ID_SIZE = 16
MSG_ID_SIZE = 4
MSG_TYPE_SIZE = 1
MSG_TYPE_MAX = 0xFF
MSG_ID_MAX = 0xFFFFFFFF
CLIENT_NAME_SIZE = 255
CLIENT_PUBLIC_KEY_SIZE = 160
REQUEST_OPTIONS = 5
RESPONSE_OPTIONS = 6


# Request Codes
class ERequestCode(Enum):
    REQUEST_REGISTRATION = 1000  # uuid ignored.
    REQUEST_USERS = 1001         # payload invalid. payloadSize = 0.
    REQUEST_PUBLIC_KEY = 1002
    REQUEST_SEND_MSG = 1003
    REQUEST_PENDING_MSG = 1004   # payload invalid. payloadSize = 0.


# Responses Codes
class EResponseCode(Enum):
    RESPONSE_REGISTRATION = 2000
    RESPONSE_USERS = 2001
    RESPONSE_PUBLIC_KEY = 2002
    RESPONSE_MSG_SENT = 2003
    RESPONSE_PENDING_MSG = 2004
    RESPONSE_ERROR = 9000  # payload invalid. payloadSize = 0.


# Message types, values are not transferred via communication.
class EMessageType(Enum):
    MSG_INVALID = DEF_VAL
    MSG_SYMMETRIC_KEY_REQUEST = 1111  # content invalid. contentSize = 0.
    MSG_SYMMETRIC_KEY = 2222          # content = symmetric key encrypted by destination client's public key.
    MSG_ENCRYPTED = 3333              # content = encrypted message by symmetric key.
    MSG_FILE = 4444                   # content = encrypted file by symmetric key.


class RequestHeader:
    def __init__(self):
        self.clientID = b""
        self.version = DEF_VAL      # 1 byte
        self.code = DEF_VAL         # 2 bytes
        self.payloadSize = DEF_VAL  # 4 bytes
        self.size = CLIENT_ID_SIZE + HEADER_SIZE

    def unpack(self, data):
        try:
            self.clientID = struct.unpack(f"<{CLIENT_ID_SIZE}s", data[:CLIENT_ID_SIZE])[0]
            headerData = data[CLIENT_ID_SIZE:CLIENT_ID_SIZE + HEADER_SIZE]
            self.version, self.code, self.payloadSize = struct.unpack("<BHL", headerData)
            return True
        except:
            self.__init__()  # reset values
            return False


class ResponseHeader:
    def __init__(self, version, code):
        self.version = version      # 1 byte
        self.code = code            # 2 bytes
        self.payloadSize = DEF_VAL  # 4 bytes
        self.size = HEADER_SIZE

    def pack(self):
        try:
            return struct.pack("<BHL", self.version, self.code, self.payloadSize)
        except:
            return b""


class RegistrationRequest:
    def __init__(self):
        self.header = RequestHeader()
        self.name = b""
        self.publicKey = b""

    def unpack(self, data):
        if not self.header.unpack(data):
            return False
        try:
            # trim the byte array after the nul terminating character.
            nameData = data[self.header.size:self.header.size+CLIENT_NAME_SIZE]
            self.name = str(struct.unpack(f"<{CLIENT_NAME_SIZE}s", nameData)[0].partition(b'\0')[0].decode('utf-8'))
            keyData = data[self.header.size+CLIENT_NAME_SIZE:self.header.size+CLIENT_NAME_SIZE+CLIENT_PUBLIC_KEY_SIZE]
            self.publicKey = struct.unpack(f"<{CLIENT_PUBLIC_KEY_SIZE}s", keyData)[0]
            return True
        except:
            self.name = b""
            self.publicKey = b""
            return False


class RegistrationResponse:
    def __init__(self, version):
        self.header = ResponseHeader(version, EResponseCode.RESPONSE_REGISTRATION.value)
        self.clientID = b""

    def pack(self):
        try:
            data = self.header.pack()
            data += struct.pack(f"<{CLIENT_ID_SIZE}s", self.clientID)
            return data
        except:
            return b""


class PublicKeyRequest:
    def __init__(self):
        self.header = RequestHeader()
        self.clientID = b""

    def unpack(self, data):
        if not self.header.unpack(data):
            return False
        try:
            clientID = data[self.header.size:self.header.size + CLIENT_ID_SIZE]
            self.clientID = struct.unpack(f"<{CLIENT_ID_SIZE}s", clientID)[0]
            return True
        except:
            self.clientID = b""
            return False


class PublicKeyResponse:
    def __init__(self, version):
        self.header = ResponseHeader(version, EResponseCode.RESPONSE_PUBLIC_KEY.value)
        self.clientID = b""
        self.publicKey = b""

    def pack(self):
        try:
            data = self.header.pack()
            data += struct.pack(f"<{CLIENT_ID_SIZE}s", self.clientID)
            data += struct.pack(f"<{CLIENT_PUBLIC_KEY_SIZE}s", self.publicKey)
            return data
        except:
            return b""


class MessageSendRequest:
    def __init__(self):
        self.header = RequestHeader()
        self.clientID = b""
        self.messageType = DEF_VAL
        self.contentSize = DEF_VAL
        self.content = b""

    def unpack(self, data):
        if not self.header.unpack(data):
            return False
        try:
            clientID = data[self.header.size:self.header.size + CLIENT_ID_SIZE]
            self.clientID = struct.unpack(f"<{CLIENT_ID_SIZE}s", clientID)[0]
            offset = self.header.size + CLIENT_ID_SIZE
            self.messageType, self.contentSize = struct.unpack("<BL", data[offset:offset+5])
            offset = self.header.size + CLIENT_ID_SIZE + 5
            self.content = struct.unpack(f"<{self.contentSize}s", data[offset:offset+self.contentSize])[0]
            return True
        except:
            self.clientID = b""
            self.messageType = DEF_VAL
            self.contentSize = DEF_VAL
            self.content = b""
            return False


class MessageSentResponse:
    def __init__(self, version):
        self.header = ResponseHeader(version, EResponseCode.RESPONSE_MSG_SENT.value)
        self.clientID = b""
        self.messageID = b""

    def pack(self):
        try:
            data = self.header.pack()
            data += struct.pack(f"<{CLIENT_ID_SIZE}sL", self.clientID, self.messageID)
            return data
        except:
            return b""