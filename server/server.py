"""
MessageU Server
Server.py: contains Server class which has socket logics.
Server class also contains the main loop of the server.
"""
__author__ = "Roman Koifman"

import selectors
import socket


class Server:
    PACKET_SIZE = 1024  # Default packet size.
    MAX_QUEUED_CONN = 5  # Default maximum number of queued connections.
    IS_BLOCKING = False

    def __init__(self, host, port):
        self.host = host
        self.port = port
        self.sel = selectors.DefaultSelector()
        self.lastErr = ""  # Last Error description.

    def accept(self, sock, mask):
        conn, address = sock.accept()
        conn.setblocking(Server.IS_BLOCKING)      # we want to avoid it. todo: setblocking(False)
        self.sel.register(conn, selectors.EVENT_READ, self.read)

    def read(self, conn, mask):
        data = conn.recv(Server.PACKET_SIZE)
        if data:
            conn.send(data)  # todo: make sure not blocking, send to dest.
        else:
            self.sel.unregister(conn)
            conn.close()

    def start(self):
        """ Start listen for connections. Contains the main loop. """
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


