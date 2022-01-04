# NetworkListener

## *(A compatible client librarie is coming soon)*

This project contains installable C++ libraries for a server (listener) on TCP level. A client can establish a connection to the server and data can be sent in both directions. The number of connected clients at one time is limited to 4096.

This packge contains two libraries: **libtcpServer** and **libtlsServer**.\
As the names say, **libtcpServer** creates a simple TCP server with no security. The **libtlsServer** creates a server on TLS level. This means, an established connection is encrypted with the latest compatible TLS version and a client is forced to authenticate itself.
