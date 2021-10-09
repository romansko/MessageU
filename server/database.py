"""
MessageU Server
database.py: handles server's database.
"""
__author__ = "Roman Koifman"


import sqlite3
import client
import message


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
        if not type(clnt) is client.Client or not clnt.validate():
            return False
        try:
            conn = self.connect()
            cur = conn.cursor()
            cur.execute(f"INSERT INTO {Database.CLIENTS} VALUES (?, ?, ?, ?)", [clnt.ID,
                                                                                clnt.Name,
                                                                                clnt.PublicKey,
                                                                                clnt.LastSeen])
            conn.commit()
            conn.close()
            return True
        except Exception as e:
            return False

    def getClientsList(self):
        conn = self.connect()
        cur = conn.cursor()
        cur.execute(f"SELECT ID, Name FROM {Database.CLIENTS}")
        clients = cur.fetchall()
        conn.close()
        return clients
