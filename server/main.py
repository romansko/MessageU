"""
MessageU Server
main.py: Entry point of MessageU Server.
https://github.com/Romansko/MessageU/blob/main/server/main.py
"""
__author__ = "Roman Koifman"

import utils
import server

if __name__ == '__main__':
    PORT_INFO = "port.info"
    port = utils.parsePort(PORT_INFO)
    if port is None:
        utils.stopServer(f"Failed to parse integer port from '{PORT_INFO}'!")
    svr = server.Server('', port)  # don't care about host.
    if not svr.start():
        utils.stopServer(f"Server start exception: {svr.lastErr}")

