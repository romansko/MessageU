"""
MessageU Server
Server.py: contains Server class which has socket logics.
Server class also contains the main loop of the server.
"""
__author__ = "Roman Koifman"

import logging
import selectors
import uuid
import socket
import utils
import database
import protocol
from datetime import datetime


class Server:
    DATABASE = 'server.db'
    PACKET_SIZE = 1024  # Default packet size.
    MAX_QUEUED_CONN = 5  # Default maximum number of queued connections.
    IS_BLOCKING = False

    def __init__(self, host, port):
        self.host = host
        self.port = port
        self.sel = selectors.DefaultSelector()
        self.database = database.Database(Server.DATABASE)
        self.lastErr = ""  # Last Error description.
        self.requestHandle = {
            protocol.ERequestCode.REQUEST_REGISTRATION.value: self.handleRegistrationRequest,
            protocol.ERequestCode.REQUEST_USERS.value: self.handleUsersListRequest,
            protocol.ERequestCode.REQUEST_PUBLIC_KEY.value: self.handlePublicKeyRequest,
            protocol.ERequestCode.REQUEST_SEND_MSG.value: self.handleMessageSendRequest,
            protocol.ERequestCode.REQUEST_PENDING_MSG.value: self.handlePendingMessagesRequest
        }

    def accept(self, sock, mask):
        conn, address = sock.accept()
        conn.setblocking(Server.IS_BLOCKING)
        self.sel.register(conn, selectors.EVENT_READ, self.read)

    def read(self, conn, mask):
        data = conn.recv(Server.PACKET_SIZE)
        if data:
            requestHeader = protocol.RequestHeader()
            if not requestHeader.unpack(data):
                logging.error("Failed to parse request header!")
            success = False
            if requestHeader.code in self.requestHandle.keys():
                success = self.requestHandle[requestHeader.code](conn, data)
            if not success:  # generic error
                responseHeader = protocol.ResponseHeader(protocol.EResponseCode.RESPONSE_ERROR.value)
                self.write(conn, responseHeader.pack())

        self.sel.unregister(conn)
        conn.close()

    def write(self, conn, data):
        """ make sure packet sent is sized """
        size = len(data)
        sent = 0
        while sent < size:
            leftover = size - sent
            if leftover > Server.PACKET_SIZE:
                leftover = Server.PACKET_SIZE
            toSend = data[sent:sent + leftover]
            if len(toSend) < Server.PACKET_SIZE:
                toSend += bytearray(Server.PACKET_SIZE - len(toSend))
            try:
                conn.send(toSend)
                sent += len(toSend)
            except:
                logging.error("Failed to send data to " + conn)
                return

    def start(self):
        """ Start listen for connections. Contains the main loop. """
        if not self.database.initialize():
            utils.stopServer("Failed to initialize " + Server.DATABASE)
        try:
            sock = socket.socket()
            sock.bind((self.host, self.port))
            sock.listen(Server.MAX_QUEUED_CONN)
            sock.setblocking(Server.IS_BLOCKING)
            self.sel.register(sock, selectors.EVENT_READ, self.accept)
        except Exception as e:
            self.lastErr = e
            return False
        print(f"Server is listening for connections on port {self.port}..")
        while True:
            events = self.sel.select()
            for key, mask in events:
                try:
                    callback = key.data
                    callback(key.fileobj, mask)
                except Exception as e:
                    print(f": Server exception handling selector event: {e}")

    def handleRegistrationRequest(self, conn, data):
        request = protocol.RegistrationRequest()
        response = protocol.RegistrationResponse()
        if not request.unpack(data):
            logging.error("Registration Request: Failed parsing request.")
            return False
        try:
            if not request.name.isalnum():
                logging.info(f"Registration Request: Invalid requested username ({request.name}))")
                return False
            if self.database.clientUsernameExists(request.name):
                logging.info(f"Registration Request: Username ({request.name}) already exists.")
                return False
        except:
            logging.error("Registration Request: Failed to connect to database.")
            return False
        clnt = database.Client(uuid.uuid4().hex, request.name, request.publicKey, str(datetime.now()))
        if not self.database.storeClient(clnt):
            logging.error("Registration Request: Failed to store client.")
            return False
        response.clientID = clnt.ID
        response.header.payloadSize = protocol.CLIENT_ID_SIZE
        self.write(conn, response.pack())
        return True

    def handleUsersListRequest(self, conn, data):
        request = protocol.RequestHeader()
        if not request.unpack(data):
            logging.error("Failed to parse request header!")
        try:
            if not self.database.clientIdExists(request.clientID):
                logging.info(f"Users list Request: clientID ({request.clientID}) does not exists!")
                return False
        except:
            logging.error("Users list Request:: Failed to connect to database.")
            return False
        response = protocol.ResponseHeader(protocol.EResponseCode.RESPONSE_USERS.value)
        clients = self.database.getClientsList()
        payload = b""
        for user in clients:
            if user[0] != request.clientID:  # Do not send self. Requirement.
                payload += user[0]
                name = user[1] + bytes('\0' * (protocol.CLIENT_NAME_SIZE - len(user[1])), 'utf-8')
                payload += name
        response.payloadSize = len(payload)
        self.write(conn, response.pack() + payload)
        return True

    def handlePublicKeyRequest(self, conn, data):
        request = protocol.PublicKeyRequest()
        response = protocol.PublicKeyResponse()
        if not request.unpack(data):
            logging.error("Failed to parse request header!")
        key = self.database.getClientPublicKey(request.clientID)
        if not key:
            logging.info(f"PublicKey Request: clientID doesn't exists.")
            return False
        response.clientID = request.clientID
        response.publicKey = key
        response.header.payloadSize = protocol.CLIENT_ID_SIZE + protocol.CLIENT_PUBLIC_KEY_SIZE
        self.write(conn, response.pack())
        return True

    def handleMessageSendRequest(self, conn, data):
        request = protocol.MessageSendRequest()
        response = protocol.MessageSentResponse()
        if not request.unpack(conn, data):
            logging.error("Failed to parse request header!")

        msg = database.Message(request.clientID,
                               request.header.clientID,
                               request.messageType,
                               request.content)

        msgId = self.database.storeMessage(msg)
        if not msgId:
            logging.error("Send Message Request: Failed to store msg.")
            return False

        response.header.payloadSize = protocol.CLIENT_ID_SIZE + protocol.MSG_ID_SIZE
        response.clientID = request.clientID
        response.messageID = msgId
        self.write(conn, response.pack())
        return True

    def handlePendingMessagesRequest(self, conn, data):
        request = protocol.RequestHeader()
        response = protocol.ResponseHeader(protocol.EResponseCode.RESPONSE_PENDING_MSG.value)
        if not request.unpack(data):
            logging.error("Failed to parse request header!")
        try:
            if not self.database.clientIdExists(request.clientID):
                logging.info(f"clientID ({request.clientID}) does not exists!")
                return False
        except:
            logging.error("Failed to connect to database.")
            return False

        payload = b""
        messages = self.database.getPendingMessages(request.clientID)
        for msg in messages:  # id, from, type, content
            pending = protocol.PendingMessage()
            pending.messageID = int(msg[0])
            pending.messageClientID = msg[1]
            pending.messageType = int(msg[2])
            pending.content = msg[3]
            pending.messageSize = len(msg[3])
            payload += pending.pack()
        response.payloadSize = len(payload)
        self.write(conn, response.pack() + payload)
        return True
