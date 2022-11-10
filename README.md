# NetworkListener

Installable package to set up a server clients can connect to on TCP level and send and receive data asynchronously. The connection can be established with unencrypted pure TCP or with encrypted and two-way-authenticated TLS.

The compatible client can be found [here](https://github.com/nilshenrich/NetworkClient)

A test run can be found [here](https://github.com/nilshenrich/NetworkTester/actions)

## Table of contents

1. [General explanation](#general-explanation)
    1. [Specifications](#specifications)
1. [Installation](#installation)
1. [Usage](#usage)
    1. [Non-abstract Methods](#non-abstract-methods)
1. [Example](#example)
    1. [Create certificates](#create-certificates)
    1. [Run example](#run-example)
1. [System requirements](#system-requirements)
1. [Known issues](#known-issues)

## General explanation

This project contains installable C++ libraries for a server (listener) on TCP level. A client can connect to the server and data can be sent in both directions.

This package contains two libraries: **libnetworkListenerTcp** and **libnetworkListenerTls**.\
As the names say, **libnetworkListenerTcp** creates a simple TCP server with no security. The **libnetworkListenerTls** creates a server on TLS level. This means, an established connection is encrypted with the latest compatible TLS version and a client is forced to authenticate itself.

### Specifications

1. Maximum number of connected clients at the same time: **4096**
1. Maximum message size (Sending and receiving):  **std::string::max_size() - 1** (2³² - 2 (4294967294) for most systems)

## Installation

As already mentioned in [General explanation](#general-explanation), this project contains two installable libraries **libnetworkListenerTcp** and **libnetworkListenerTcp**. These libraries can be installed this way:

1. Install **build-essential**, **cmake**, **openssl** and **libssl-dev**:

    ```console
    sudo apt install build-essential cmake openssl libssl-dev
    ```

    - **build-essential** contains compilers and libraries to compile C/C++ code on debian-based systems.
    - **cmake** is used to easily create a Makefile.
    - **openssl** is a great toolbox for any kind of tasks about encryption and certification. It is here used to create certificates and keys used for encryption with TLS.
    - **libssl-dev** is the openssl library for C/C++.

1. Create a new folder **build** in the repositories root directory where everything can be compiled to:

    ```console
    mkdir build
    ```

1. Navigate to this newly created **build** folder and run the compilation process:

    ```console
    cd build
    cmake ..
    make
    ```

    *To get information printed on the screen, the package can be built in debug mode (Not recommended when installing libraries) by setting the define: **cmake&#160;&#x2011;DCMAKE_BUILD_TYPE=Debug&#160;..** (Same when compiling example)*

1. To install the created libraries and header files to your system, run

    ```console
    sudo make install
    ```

1. (optional) In some systems the library paths need to be updated before **libnetworkListenerTcp** and **libnetworkListenerTls** can be used. So if you get an error like

    ```console
    ./example: error while loading shared libraries: libnetworkListenerTcp.so.1: cannot open shared object file: No such file or directory
    ```

    please run

    ```console
    sudo /sbin/ldconfig
    ```

## Usage

*In the subfolder [example](https://github.com/nilshenrich/NetworkListener/blob/main/example/main.cpp) you can find a good and simple example program that shows how to use the package*

To use this package, a new class must be created deriving from **TcpServer** or **TlsServer** or both. These two classes are abstract, so an object of one of these raw types can't be created.

In this case, I would recommend a private derivation, because all **TcpServer**/**TlsServer** methods are not meant to be used in other places than a direct child class.

1. Create a new class derived from **TcpServer** and **TlsServer**:

    ```cpp
    #include "NetworkListener/TcpServer.h"
    #include "NetworkListener/TlsServer.h"

    using namespace std;
    using namespace networking;

    // New class with TCP and TLS listener functionality
    class ExampleServer : private TcpServer, private TlsServer
    {
    public:
        // Constructor and destructor
        ExampleServer() {}
        virtual ~ExampleServer() {}
    };
    ```

1. Implement abstract methods from base classes
    1. Work on message over TCP (unencrypted):

        ```cpp
        void workOnMessage_TcpServer(const int tcpClientId, const std::string tcpMsgFromClient)
        {
            // Do some stuff when message is received
        }
        ```

        This method is called automatically as soon as a new message from a client is received over an unencrypted TCP connection.

        Parameters:
        - The **tcpClientId** parameter is the TCP ID of the sending client. This number is used to identify connected clients.

        - The **tcpMsgFromClient** parameter contains the received message as raw string.

        This method is started in its own thread. So feel free to insert time intensive code here, the listener continues running in parallel. But please make sure, your inserted code is thread safe (e.g. use [mutex](https://en.cppreference.com/w/cpp/thread/mutex)), so multiple executions of this method at the same time don't lead to a program crash.

    1. Work on a closed TCP connection:

        ```cpp
        workOnClosed_TcpServer(const int tcpClientId)
        {
            // Do some stuff when existing TCP connection is closed
        }
        ```

        This method is called as soon as an established TCP to a client is closed. When this method has finished, the TCP socket gets completely closed to have it open again for new connections.

        The **tcpClientId** parameter is the TCP ID of the client whose connection is closed. This number is used to identify connected clients.

    1. Work on message over TLS (encrypted):

        ```cpp
        void workOnMessage_TlsServer(const int tlsClientId, const std::string tlsMsgFromClient)
        {
            // Do some stuff when message is received
        }
        ```

        This method is just the same as **workOnMessage_TcpServer**, but for receiving an encrypted message over a TLS connection.

    1. Work on a closed TLS connection:

        ```cpp
        void workOnClosed_TlsServer(const int tlsClientId)
        {
            // Do some stuff when existing TLS connection is closed
        }
        ```

        This method is just the same as **workOnClosed_TcpServer**, but for a closed TLS connection.

    **!! Please do never call one of these 4 abstract methods somewhere in your code. These methods are automatically called by the NetworkListener library.**

    *Please note that all parameters of these abstract methods are **const**, so they can't be changed. If you need to do a message adaption, but don't want to copy the whole string for performance reasons, use the **move**-constructor:*

    ```cpp
    std::string modifiable = std::move(tcpMsgFromClient);
    modifiable += '\n'; // Message modification
    ```

After these two steps your program is ready to be compiled.\
But there are some further methods worth knowing about.

### Non-abstract Methods

1. start():

    The **start**-method is used to start a TCP or TLS listener. When this method returns 0, the listener runs in the background. If the return value is other that 0, please see [NetworkingDefines.h](https://github.com/nilshenrich/NetworkListener/blob/main/include/NetworkingDefines.h) for definition of error codes.\
    If your class derived from both **TcpServer** and **TlsServer**, the class name must be specified when calling **start()**:

    ```cpp
    TcpServer::start(8081);
    TlsServer::start(8082, "ca_cert.pem", "server_cert.pem", "server_key.pem");
    ```

1. stop():

    The **stop**-method stops a running listener.\
    As for **start()**, if your class derived from both **TcpServer** and **TlsServer**, the class name must be specified when calling **stop()**:

    ```cpp
    TcpServer::stop();
    TlsServer::stop();
    ```

1. sendMsg():

    The **sendMsg**-method sends a message to a connected client (over TCP or TLS). If the return value is **true**, the sending was successful, if it is **false**, not.\
    As for **start()**, if your class derived from both **TcpServer** and **TlsServer**, the class name must be specified when calling **sendMsg()**:

    ```cpp
    TcpServer::sendMsg(4, "example message over TCP");
    TlsServer::sendMsg(5, "example message over TLS");
    ```

    Please make sure to only use **TcpServer::sendMsg()** for TCP connections and **TlsServer::sendMsg()** for TLS connection.

1. getClientIp():

    The **getClientIp**-method returns the IP address of a connected client (TCP or TLS) identified by its TCP ID. If no client with this ID is connected, the string **"Failed Read!"** is returned.

1. TlsServer::getSubjPartFromClientCert():

    The **getSubjPartFromClientCert**-method only exists for **TlsServer** and returns a given subject part of the client's certificate identified by its TCP ID or its tlsSocket (SSL*). If the tlsSocket parameter is*nullptr*, the client is identified by its TCP ID, otherwise it is identified by the given tlsSocket parameter.

    The subject of a certificate contains information about the certificate owner. Here is a list of all subject parts and how to get them using **getSubjPartFromClientCert()**:

    - **NID_countryName**: Country code (Germany = DE)
    - **NID_stateOrProvinceName**: State or province name (e.g. Baden-Württemberg)
    - **NID_localityName**: City name (e.g. Stuttgart)
    - **NID_organizationName**: Organization name
    - **NID_organizationalUnitName**: Organizational unit name
    - **NID_commonName**: Name of owner

    For example

    ```cpp
    getSubjPartFromClientCert(4, nullptr, NID_localityName);
    ```

    will return "Stuttgart" if this is the client's city name.

1. isRunning():

    The **isRunning**-method returns the running flag of the NetworkListener.\
    **True** means: *The listener is running*\
    **False** means: *The listener is not running*

## Example

This repository contains a small example to show the usage of this package. It creates two listeners, one using unsecure TCP, the other using encrypted and two-way-authenticated TLS (two-way authentication means, the server authenticates itself with a CA-signed certificate ad forces the client to also authenticate itself with his own CA-signed certificate).\
The example program runs for 10 seconds. Within this time range, it can accept new client connections and receive data. Received data will be printed on the screen and sent back to the sending client.

### Create certificates

Before the encrypted TLS listener can run properly, the needed certificates and private keys need to be created. To do this, just run the bash file

```console
example/CMakeLists.txt
```

### Run example

The example can be compiled the same way as the libraries (Without installing at the end):

```console
cd example
mkdir build
cd build
cmake ..
make
./example
```

## System requirements

Linux distribution based on debian buster or later.

The installation process in this project is adapted to debian-based linux distributions. But smart guys maybe achieve to make it usable on other systems (In the end it is just C++ code compilable with C++17 standard or higher).

## Known issues

\<no known issues\>
