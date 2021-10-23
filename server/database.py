"""
MessageU Server
database.py: handles server's database.
https://github.com/Romansko/MessageU/blob/main/server/database.py
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
        if not self.Name or len(self.Name) >= protocol.NAME_SIZE:
            return False
        if not self.PublicKey or len(self.PublicKey) != protocol.PUBLIC_KEY_SIZE:
            return False
        if not self.LastSeen:
            return False
        return True


class Message:
    """ Represents a message entry """

    def __init__(self, to_client, from_client, mtype, content):
        self.ID = 0  # Message ID, 4 bytes.
        self.ToClient = to_client  # Receiver's unique ID, 16 bytes.
        self.FromClient = from_client  # Sender's unique ID, 16 bytes.
        self.Type = mtype  # Message type, 1 byte.
        self.Content = content  # Message's content, Blob.

    def validate(self):
        """ Validate Message attributes according to the requirements """
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
        conn = sqlite3.connect(self.name)  # doesn't raise exception.
        conn.text_factory = bytes
        return conn

    def executescript(self, script):
        conn = self.connect()
        try:
            conn.executescript(script)
            conn.commit()
        except:
            pass
        conn.close()

    def execute(self, query, args, commit=False, get_last_row=False):
        """ Given an query and args, execute query, and return the results. """
        results = None
        conn = self.connect()
        try:
            cur = conn.cursor()
            cur.execute(query, args)
            if commit:
                conn.commit()
                results = True
            else:
                results = cur.fetchall()
            if get_last_row:
                results = cur.lastrowid  # special query.
        except Exception as e:
            pass
        conn.close()  # commit is not required.
        return results

    def initialize(self):
        # Try to create Clients table
        self.executescript(f"""
            CREATE TABLE {Database.CLIENTS}(
              ID CHAR(16) NOT NULL PRIMARY KEY,
              Name CHAR(255) NOT NULL,
              PublicKey CHAR(160) NOT NULL,
              LastSeen DATE
            );
            """)

        # Try to create Messages table
        self.executescript(f"""
            CREATE TABLE {Database.MESSAGES}(
              ID INTEGER PRIMARY KEY,
              ToClient CHAR(16) NOT NULL,
              FromClient CHAR(16) NOT NULL,
              Type CHAR(1) NOT NULL,
              Content BLOB,
              FOREIGN KEY(ToClient) REFERENCES {Database.CLIENTS}(ID),
              FOREIGN KEY(FromClient) REFERENCES {Database.CLIENTS}(ID)
            );
            """)


    def clientUsernameExists(self, username):
        """ Check whether a username already exists within database """
        results = self.execute(f"SELECT * FROM {Database.CLIENTS} WHERE Name = ?", [username])
        if not results:
            return False
        return len(results) > 0

    def clientIdExists(self, client_id):
        """ Check whether an client ID already exists within database """
        results = self.execute(f"SELECT * FROM {Database.CLIENTS} WHERE ID = ?", [client_id])
        if not results:
            return False
        return len(results) > 0

    def storeClient(self, clnt):
        """ Store a client into database """
        if not type(clnt) is Client or not clnt.validate():
            return False
        return self.execute(f"INSERT INTO {Database.CLIENTS} VALUES (?, ?, ?, ?)",
                            [clnt.ID, clnt.Name, clnt.PublicKey, clnt.LastSeen], True)

    def storeMessage(self, msg):
        """ Store a message into database """
        if not type(msg) is Message or not msg.validate():
            return False
        results = self.execute(
            f"INSERT INTO {Database.MESSAGES}(ToClient, FromClient, Type, Content) VALUES (?, ?, ?, ?)",
            [msg.ToClient, msg.FromClient, msg.Type, msg.Content], True, True)
        return results

    def removeMessage(self, msg_id):
        """ remove a message by id from database """
        return self.execute(f"DELETE FROM {Database.MESSAGES} WHERE ID = ?", [msg_id], True)

    def setLastSeen(self, client_id, time):
        """ set last seen given a client_id """
        return self.execute(f"UPDATE {Database.CLIENTS} SET LastSeen = ? WHERE ID = ?",
                            [time, client_id], True)

    def getClientsList(self):
        """ query for all clients """
        return self.execute(f"SELECT ID, Name FROM {Database.CLIENTS}", [])

    def getClientPublicKey(self, client_id):
        """ given a client id, return a public key. """
        results = self.execute(f"SELECT PublicKey FROM {Database.CLIENTS} WHERE ID = ?", [client_id])
        if not results:
            return None
        return results[0][0]

    def getPendingMessages(self, client_id):
        """ given a client id, return pending messages for that client. """
        return self.execute(f"SELECT ID, FromClient, Type, Content FROM {Database.MESSAGES} WHERE ToClient = ?",
                            [client_id])
