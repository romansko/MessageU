"""
MessageU Server
utils.py: collection of small functions which make MessageU server's patterns shorter & easier.
https://github.com/Romansko/MessageU/blob/main/server/utils.py
"""
__author__ = "Roman Koifman"


def stopServer(err):
    """ Print err and stop script execution """
    print(f"\nFatal Error: {err}\nMessageU Server will halt!")
    exit(1)


def parsePort(filepath):
    """
    Parse filepath for port number. Return port as integer.
    Note: Only the 1st line will be read. On any failure, None will be returned.
    """
    port = None
    try:
        with open(filepath, "r") as port_info:
            port = port_info.readline().strip()
            port = int(port)
    except (ValueError, FileNotFoundError):
        port = None
    finally:
        return port
