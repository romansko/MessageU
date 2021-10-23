"""
MessageU Server
protocol.py: defines protocol structs and constants.
https://github.com/Romansko/MessageU/blob/main/server/protocol.py
"""
__author__ = "Roman Koifman"

import struct
from enum import Enum

SERVER_VERSION = 2    # Ver2 - support SQL Database.
DEF_VAL = 0           # Default value to initialize inner fields.
HEADER_SIZE = 7       # Header size without clientID. (version, code, payload size).
CLIENT_ID_SIZE = 16
MSG_ID_SIZE = 4
MSG_TYPE_MAX = 0xFF
MSG_ID_MAX = 0xFFFFFFFF
NAME_SIZE = 255
PUBLIC_KEY_SIZE = 160


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
    RESPONSE_ERROR = 9000        # payload invalid. payloadSize = 0.


class RequestHeader:
    def __init__(self):
        self.clientID = b""
        self.version = DEF_VAL      # 1 byte
        self.code = DEF_VAL         # 2 bytes
        self.payloadSize = DEF_VAL  # 4 bytes
        self.SIZE = CLIENT_ID_SIZE + HEADER_SIZE

    def unpack(self, data):
        """ Little Endian unpack Request Header """
        try:
            self.clientID = struct.unpack(f"<{CLIENT_ID_SIZE}s", data[:CLIENT_ID_SIZE])[0]
            headerData = data[CLIENT_ID_SIZE:CLIENT_ID_SIZE + HEADER_SIZE]
            self.version, self.code, self.payloadSize = struct.unpack("<BHL", headerData)
            return True
        except:
            self.__init__()  # reset values
            return False


class ResponseHeader:
    def __init__(self, code):
        self.version = SERVER_VERSION  # 1 byte
        self.code = code               # 2 bytes
        self.payloadSize = DEF_VAL     # 4 bytes
        self.SIZE = HEADER_SIZE

    def pack(self):
        """ Little Endian pack Response Header """
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
        """ Little Endian unpack Request Header and Registration data """
        if not self.header.unpack(data):
            return False
        try:
            # trim the byte array after the nul terminating character.
            nameData = data[self.header.SIZE:self.header.SIZE + NAME_SIZE]
            self.name = str(struct.unpack(f"<{NAME_SIZE}s", nameData)[0].partition(b'\0')[0].decode('utf-8'))
            keyData = data[self.header.SIZE + NAME_SIZE:self.header.SIZE + NAME_SIZE + PUBLIC_KEY_SIZE]
            self.publicKey = struct.unpack(f"<{PUBLIC_KEY_SIZE}s", keyData)[0]
            return True
        except:
            self.name = b""
            self.publicKey = b""
            return False


class RegistrationResponse:
    def __init__(self):
        self.header = ResponseHeader(EResponseCode.RESPONSE_REGISTRATION.value)
        self.clientID = b""

    def pack(self):
        """ Little Endian pack Response Header and client ID """
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
        """ Little Endian unpack Request Header and client ID """
        if not self.header.unpack(data):
            return False
        try:
            clientID = data[self.header.SIZE:self.header.SIZE + CLIENT_ID_SIZE]
            self.clientID = struct.unpack(f"<{CLIENT_ID_SIZE}s", clientID)[0]
            return True
        except:
            self.clientID = b""
            return False


class PublicKeyResponse:
    def __init__(self):
        self.header = ResponseHeader(EResponseCode.RESPONSE_PUBLIC_KEY.value)
        self.clientID = b""
        self.publicKey = b""

    def pack(self):
        """ Little Endian pack Response Header and Public Key """
        try:
            data = self.header.pack()
            data += struct.pack(f"<{CLIENT_ID_SIZE}s", self.clientID)
            data += struct.pack(f"<{PUBLIC_KEY_SIZE}s", self.publicKey)
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

    def unpack(self, conn, data):
        """ Little Endian unpack Request Header and message data """
        packetSize = len(data)
        if not self.header.unpack(data):
            return False
        try:
            clientID = data[self.header.SIZE:self.header.SIZE + CLIENT_ID_SIZE]
            self.clientID = struct.unpack(f"<{CLIENT_ID_SIZE}s", clientID)[0]
            offset = self.header.SIZE + CLIENT_ID_SIZE
            self.messageType, self.contentSize = struct.unpack("<BL", data[offset:offset + 5])
            offset = self.header.SIZE + CLIENT_ID_SIZE + 5
            bytesRead = packetSize - offset
            if bytesRead > self.contentSize:
                bytesRead = self.contentSize
            self.content = struct.unpack(f"<{bytesRead}s", data[offset:offset + bytesRead])[0]
            while bytesRead < self.contentSize:
                data = conn.recv(packetSize)  # reuse first size of data.
                dataSize = len(data)
                if (self.contentSize - bytesRead) < dataSize:
                    dataSize = self.contentSize - bytesRead
                self.content += struct.unpack(f"<{dataSize}s", data[:dataSize])[0]
                bytesRead += dataSize
            return True
        except:
            self.clientID = b""
            self.messageType = DEF_VAL
            self.contentSize = DEF_VAL
            self.content = b""
            return False


class MessageSentResponse:
    def __init__(self):
        self.header = ResponseHeader(EResponseCode.RESPONSE_MSG_SENT.value)
        self.clientID = b""
        self.messageID = b""

    def pack(self):
        """ Little Endian pack Response Header and client ID """
        try:
            data = self.header.pack()
            data += struct.pack(f"<{CLIENT_ID_SIZE}sL", self.clientID, self.messageID)
            return data
        except:
            return b""


class PendingMessage:
    def __init__(self):
        self.messageClientID = b""
        self.messageID = 0
        self.messageType = 0
        self.messageSize = 0
        self.content = b""

    def pack(self):
        try:
            """ Little Endian pack Response Header and pending message header """
            data = struct.pack(f"<{CLIENT_ID_SIZE}s", self.messageClientID)
            data += struct.pack("<LBL", self.messageID, self.messageType, self.messageSize)
            data += struct.pack(f"<{self.messageSize}s", self.content)
            return data
        except:
            return b""


