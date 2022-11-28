# NetworkListener

Installable package to set up a server clients can connect to on TCP level and send and receive data asynchronously. The connection can be established with unencrypted pure TCP or with encrypted and two-way-authenticated TLS.

The compatible client can be found [here](https://github.com/nilshenrich/NetworkClient)

A test run can be found [here](https://github.com/nilshenrich/NetworkTester/actions)

## Table of contents

1. [General explanation](#general-explanation)
    1. [Specifications](#specifications)
1. [Installation](#installation)
1. [Usage](#usage)
    1. [Preparation](#preparation)
    1. [Methods](#methods)
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

*In the subfolder [example](example/main.cpp) you can find a good and simple example program that shows how to use the package*

### Preparation

To use this package, just create an instance of **TcpServer** or **TlsServer** by using one of the provided constructors.\
For the data transfer, either the **fragmentation-mode** or the **forwarding-mode** can be chosen.\
In **fragmentation-mode**, a delimiter character must defined to split the incoming data stream to explicit messages. Please note that when using this mode, the delimiter character can't be part of any message.\
In **forwarding-mode**, all incoming data gets forwarded to an output stream of your choice. I recommend to use the append-mode when defining this output stream.

1. Implement worker methods

    ```cpp
    // Worker for incoming message (Only used in fragmentation-mode)
    void worker_message(int clientId, string msg)
    {
        // Do stuff with message
        // (clientId and msg could be changed if needed)
    }

    // Worker for closed connection to client
    void worker_closed(int clientId)
    {
        // Do stuff after closing connection
        // (clientId could be changed if needed)
    }

    // Output stream generator
    ofstream *genertor_outStream(int clientId)
    {
        // Stream must be generated with new
        // This example uses file stream but any other ostream could be used
        return new ofstream{"FileForClient_"s + to_string(clientId), ios::app};
    }
    ```

1. Create instance

    ```cpp
    // Fragmentation mode (Delimiter is line break in this case)
    TcpServer tcp_fragm{'\n', &worker_message, &worker_closed};
    TlsServer tls_fragm{'\n', &worker_message, &worker_closed};

    // Forwarding mode
    TcpServer tcp_fwd{&worker_closed, &genertor_outStream};
    TlsServer tls_fwd{&worker_closed, &genertor_outStream};
    ```

### Methods

All methods can be used the same way for **fragmentation-mode** or **forwarding-mode**.

1. start():

    The **start**-method is used to start a TCP or TLS listener. When this method returns 0, the listener runs in the background. If the return value is other that 0, please see [NetworkingDefines.h](include/NetworkingDefines.h) for definition of error codes.\
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
