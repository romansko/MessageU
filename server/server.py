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
import client
import utils
import database
import protocol
from datetime import datetime

class Server:
    VERSION = 2
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
                responseHeader = protocol.ResponseHeader(Server.VERSION)
                responseHeader.code = protocol.EResponseCode.RESPONSE_ERROR.value
                self.write(conn, responseHeader.pack())

        self.sel.unregister(conn)
        conn.close()

    def write(self, conn, data):
        """ make sure packet sent is sized """
        size = len(data)
        if size < Server.PACKET_SIZE:
            data += bytearray(Server.PACKET_SIZE - size)  # append to PACKET_SIZE bytes
        elif size > Server.PACKET_SIZE:
            data = data[:Server.PACKET_SIZE]
        try:
            conn.send(data)
        except:
            logging.error("Failed to send data to " + conn)

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
        response = protocol.RegistrationResponse(Server.VERSION)
        if not request.unpack(data):
            logging.error("Registration Request: Failed parsing request.")
            return False
        try:
            if not request.name.isalnum():
                logging.info(f"Registration Request: Invalid requested username ({request.name}))")
                return False
            if self.database.clientExists(request.name):
                logging.info(f"Registration Request: Username ({request.name}) already exists.")
                return False
        except:
            logging.error("Registration Request: Failed to connect to database.")
            return False
        clnt = client.Client(uuid.uuid4().hex, request.name, request.publicKey, str(datetime.now()))
        if not self.database.storeClient(clnt):
            logging.error("Registration Request: Failed to store client.")
            return False
        response.clientID = clnt.ID
        response.header.payloadSize = protocol.CLIENT_ID_SIZE
        self.write(conn, response.pack())
        return True


    def handleUsersListRequest(self, conn, data):
        return True

    def handlePublicKeyRequest(self, conn, data):
        return True

    def handleMessageSendRequest(self, conn, data):
        return True

    def handlePendingMessagesRequest(self, conn, data):
        return True
