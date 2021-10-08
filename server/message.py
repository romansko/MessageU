"""
MessageU Server
message.py: contains Message class which represents a message entry.
"""
__author__ = "Roman Koifman"


class Message:
    def __init__(self, msg_id, to_client, from_client, mtype, content):
        self.ID = msg_id               # Message ID, bytes.
        self.ToClient = to_client      # Receiver's unique ID, 16 bytes.
        self.FromClient = from_client  # Sender's unique ID, 16 bytes.
        self.Type = mtype              # Message type, 1 byte.
        self.Content = content         # Message's content, Blob.

    def validate(self):
        """ Validate Message attributes according to the requirements """
        pass
