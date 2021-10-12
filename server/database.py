"""
MessageU Server
database.py: handles server's database.
"""
__author__ = "Roman Koifman"

import sqlite3
import protocol


class Client:
    """ Represents a client entry """

    def __init__(self, cid, cname, public_key, last_seen):
        self.ID = bytes.fromhex(cid)  # Unique client ID, 16 bytes.
        self.Name = cname  # Client's name, null terminated ascii string, 255 bytes.
        self.PublicKey = public_key  # Client's public key, 160 bytes.
        self.LastSeen = last_seen  # The Date & time of client's last request.

    def validate(self):
        """ Validate Client attributes according to the requirements """
        if not self.ID or len(self.ID) != protocol.CLIENT_ID_SIZE:
            return False
        if not self.Name or len(self.Name) >= protocol.CLIENT_NAME_SIZE:
            return False
        if not self.PublicKey or len(self.PublicKey) != protocol.CLIENT_PUBLIC_KEY_SIZE:
            return False
        if not self.LastSeen:
            return False
        return True


class Message:
    """ Represents a message entry """

    def __init__(self, msg_id, to_client, from_client, mtype, content):
        self.ID = msg_id  # Message ID, bytes.
        self.ToClient = to_client  # Receiver's unique ID, 16 bytes.
        self.FromClient = from_client  # Sender's unique ID, 16 bytes.
        self.Type = mtype  # Message type, 1 byte.
        self.Content = content  # Message's content, Blob.

    def validate(self):
        """ Validate Message attributes according to the requirements """
        if not self.ID or self.ID > protocol.MSG_ID_MAX:
            return False
        if not self.ToClient or len(self.ToClient) != protocol.CLIENT_ID_SIZE:
            return False
        if not self.FromClient or len(self.FromClient) != protocol.CLIENT_ID_SIZE:
            return False
        if not self.Type or self.Type > protocol.MSG_TYPE_MAX:
            return False
        return True


class Database:
    CLIENTS = 'clients'
    MESSAGES = 'messages'

    def __init__(self, name):
        self.name = name

    def connect(self):
        conn = sqlite3.connect(self.name)
        conn.text_factory = bytes
        return conn

    def initialize(self):
        conn = self.connect()
        commit = False
        try:
            conn.executescript(f"""
            CREATE TABLE {Database.CLIENTS}(
              ID CHAR(16) NOT NULL PRIMARY KEY,
              Name CHAR(255) NOT NULL,
              PublicKey CHAR(160) NOT NULL,
              LastSeen DATE
            );
            """)
            commit = True
        except:
            pass  # Table possibly already exists
        try:
            conn.executescript(f"""
            CREATE TABLE {Database.MESSAGES}(
              ID INT NOT NULL PRIMARY KEY,
              ToClient CHAR(16) NOT NULL,
              FromClient CHAR(16) NOT NULL,
              Type CHAR(1) NOT NULL,
              Content BLOB,
              FOREIGN KEY(ToClient) REFERENCES {Database.CLIENTS}(ID),
              FOREIGN KEY(FromClient) REFERENCES {Database.CLIENTS}(ID)
            );
            """)
            commit = True
        except:
            pass  # Table possibly already exists
        try:
            if commit:
                conn.commit()
            conn.close()
            return True
        except:
            return False

    def clientUsernameExists(self, username):
        """ Check whether a username already exists within database """
        conn = self.connect()
        cur = conn.cursor()
        cur.execute(f"SELECT * FROM {Database.CLIENTS} WHERE Name = ?", [username])
        exists = len(cur.fetchall()) > 0
        conn.close()
        return exists

    def clientIdExists(self, id):
        """ Check whether an client ID already exists within database """
        conn = self.connect()
        cur = conn.cursor()
        cur.execute(f"SELECT * FROM {Database.CLIENTS} WHERE ID = ?", [id])
        exists = len(cur.fetchall()) > 0
        conn.close()
        return exists

    def storeClient(self, clnt):
        if not type(clnt) is Client or not clnt.validate():
            return False
        try:
            conn = self.connect()
            cur = conn.cursor()
            cur.execute(f"INSERT INTO {Database.CLIENTS} VALUES (?, ?, ?, ?)",
                        [clnt.ID, clnt.Name, clnt.PublicKey, clnt.LastSeen])
            conn.commit()
            conn.close()
            return True
        except Exception as e:
            return False

    def storeMessage(self, msg):
        if not type(msg) is Message or not msg.validate():
            return False
        try:
            conn = self.connect()
            cur = conn.cursor()
            cur.execute(f"INSERT INTO {Database.MESSAGES} VALUES (?, ?, ?, ?, ?)",
                        [msg.ID, msg.ToClient, msg.FromClient, msg.Type, msg.Content])
            conn.commit()
            conn.close()
            return True
        except:
            return False

    def getClientsList(self):
        try:
            conn = self.connect()
            cur = conn.cursor()
            cur.execute(f"SELECT ID, Name FROM {Database.CLIENTS}")
            clients = cur.fetchall()
            conn.close()
            return clients
        except:
            return []

    def getClientPublicKey(self, client_id):
        try:
            conn = self.connect()
            cur = conn.cursor()
            cur.execute(f"SELECT PublicKey FROM {Database.CLIENTS} WHERE ID = ?", [client_id])
            key = cur.fetchall()[0][0]
            conn.close()
            return key
        except:
            return []

