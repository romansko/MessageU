"""
MessageU Server
client.py: contains Client class which represents a client entry.
"""
__author__ = "Roman Koifman"


class Client:
    def __init__(self, cid, cname, public_key, last_seen):
        self.ID = cid                # Unique client ID, 16 bytes.
        self.Name = cname            # Client's name, null terminated ascii string, 255 bytes.
        self.PublicKey = public_key  # Client's public key, 160 bytes.
        self.LastSeen = last_seen    # The Date & time of client's last request.

    def validate(self):
        """ Validate Client attributes according to the requirements """
        pass

