"""
MessageU Server
Server.py: contains Server class which has socket logics. Contains main loop of the server.
https://github.com/Romansko/MessageU/blob/main/server/server.py
"""
__author__ = "Roman Koifman"

import logging
import selectors
import uuid
import socket
import database
import protocol
from datetime import datetime


class Server:
    DATABASE = 'server.db'
    PACKET_SIZE = 1024   # Default packet size.
    MAX_QUEUED_CONN = 5  # Default maximum number of queued connections.
    IS_BLOCKING = False  # Do not block!

    def __init__(self, host, port):
        """ Initialize server. Map request codes to handles. """
        logging.basicConfig(format='[%(levelname)s - %(asctime)s]: %(message)s', level=logging.INFO, datefmt='%H:%M:%S')
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
        """ accept a connection from client """
        conn, address = sock.accept()
        conn.setblocking(Server.IS_BLOCKING)
        self.sel.register(conn, selectors.EVENT_READ, self.read)

    def read(self, conn, mask):
        """ read data from client and parse it"""
        logging.info("A client has connected.")
        data = conn.recv(Server.PACKET_SIZE)
        if data:
            requestHeader = protocol.RequestHeader()
            success = False
            if not requestHeader.unpack(data):
                logging.error("Failed to parse request header!")
            else:
                if requestHeader.code in self.requestHandle.keys():
                    success = self.requestHandle[requestHeader.code](conn, data)  # invoke corresponding handle.
            if not success:  # return generic error upon failure.
                responseHeader = protocol.ResponseHeader(protocol.EResponseCode.RESPONSE_ERROR.value)
                self.write(conn, responseHeader.pack())
            self.database.setLastSeen(requestHeader.clientID, str(datetime.now()))
        self.sel.unregister(conn)
        conn.close()

    def write(self, conn, data):
        """ Send a response to client"""
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
                logging.error("Failed to send response to " + conn)
                return False
        logging.info("Response sent successfully.")
        return True

    def start(self):
        """ Start listen for connections. Contains the main loop. """
        self.database.initialize()
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
            try:
                events = self.sel.select()
                for key, mask in events:
                    callback = key.data
                    callback(key.fileobj, mask)
            except Exception as e:
                logging.exception(f"Server main loop exception: {e}")

    def handleRegistrationRequest(self, conn, data):
        """ Register a new user. Save to db. """
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
            logging.error(f"Registration Request: Failed to store client {request.name}.")
            return False
        logging.info(f"Successfully registered client {request.name}.")
        response.clientID = clnt.ID
        response.header.payloadSize = protocol.CLIENT_ID_SIZE
        return self.write(conn, response.pack())

    def handleUsersListRequest(self, conn, data):
        """ Respond with clients list to user request """
        request = protocol.RequestHeader()
        if not request.unpack(data):
            logging.error("Users list Request: Failed to parse request header!")
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
                name = user[1] + bytes('\0' * (protocol.NAME_SIZE - len(user[1])), 'utf-8')
                payload += name
        response.payloadSize = len(payload)
        logging.info(f"Clients list was successfully built for clientID ({request.clientID}).")
        return self.write(conn, response.pack() + payload)

    def handlePublicKeyRequest(self, conn, data):
        """ respond with public key of requested user id """
        request = protocol.PublicKeyRequest()
        response = protocol.PublicKeyResponse()
        if not request.unpack(data):
            logging.error("PublicKey Request: Failed to parse request header!")
        key = self.database.getClientPublicKey(request.clientID)
        if not key:
            logging.info(f"PublicKey Request: clientID doesn't exists.")
            return False
        response.clientID = request.clientID
        response.publicKey = key
        response.header.payloadSize = protocol.CLIENT_ID_SIZE + protocol.PUBLIC_KEY_SIZE
        logging.info(f"Public Key response was successfully built to clientID ({request.header.clientID}).")
        return self.write(conn, response.pack())

    def handleMessageSendRequest(self, conn, data):
        """ store a message from one user to another """
        request = protocol.MessageSendRequest()
        response = protocol.MessageSentResponse()
        if not request.unpack(conn, data):
            logging.error("Send Message Request: Failed to parse request header!")

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
        logging.info(f"Message from clientID ({request.header.clientID}) successfully stored.")
        return self.write(conn, response.pack())

    def handlePendingMessagesRequest(self, conn, data):
        """ respond with pending messages """
        request = protocol.RequestHeader()
        response = protocol.ResponseHeader(protocol.EResponseCode.RESPONSE_PENDING_MSG.value)
        if not request.unpack(data):
            logging.error("Pending messages request: Failed to parse request header!")
        try:
            if not self.database.clientIdExists(request.clientID):
                logging.info(f"clientID ({request.clientID}) does not exists!")
                return False
        except:
            logging.error("Pending messages request: Failed to connect to database.")
            return False

        payload = b""
        messages = self.database.getPendingMessages(request.clientID)
        ids = []
        for msg in messages:  # id, from, type, content
            pending = protocol.PendingMessage()
            pending.messageID = int(msg[0])
            pending.messageClientID = msg[1]
            pending.messageType = int(msg[2])
            pending.content = msg[3]
            pending.messageSize = len(msg[3])
            ids += [pending.messageID]
            payload += pending.pack()
        response.payloadSize = len(payload)
        logging.info(f"Pending messages to clientID ({request.clientID}) successfully extracted.")
        if self.write(conn, response.pack() + payload):
            for msg_id in ids:
                self.database.removeMessage(msg_id)
            return True
        return False
