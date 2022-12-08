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
    1. [Passing worker function](#passing-worker-function)
1. [Example](#example)
    1. [Create certificates](#create-certificates)
    1. [Run example](#run-example)
1. [System requirements](#system-requirements)
1. [Known issues](#known-issues)
    1. [Pipe error if client sends immediately after exiting start](#pipe-error-if-client-sends-immediately-after-exiting-start)

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

Please mind that the TcpServer and TlsServer are defined in namespace ```networking```, so all example code here expects this using directive:

```cpp
using namespace networking;
```

### Preparation

To use this package, just create an instance of **TcpServer** or **TlsServer** by using one of the provided constructors.\
For the data transfer, either the **fragmentation-mode** or the **forwarding-mode** can be chosen.\
In **fragmentation-mode**, a delimiter character must defined to split the incoming data stream to explicit messages. Please note that when using this mode, the delimiter character can't be part of any message.\
In **forwarding-mode**, all incoming data gets forwarded to an output stream of your choice. I recommend to use the normal writing mode when defining this output stream.

Worker functions can be defined that are executed automatically on specific events. For more details, please check [Passing worker function](#passing-worker-function).

1. Implement worker methods

    ```cpp
    // Worker for incoming message (Only used in fragmentation-mode)
    void worker_message(int clientId, string msg)
    {
        // Do stuff with message
        // (clientId and msg could be changed if needed)
    }

    // Worker for established connection to a client
    void worker_established(int clientId)
    {
        // Do stuff immediately after establishing a new connection
        // (clientId could be changed if needed)
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
        // (clientId could be changed if needed)
        return new ofstream{"FileForClient_"s + to_string(clientId)};
    }
    ```

    Please be aware that the ```stop``` method must not be used in any worker function. This would lead to a program stuck.

1. Create instance

    ```cpp
    // Fragmentation mode (Delimiter is line break in this case)
    TcpServer tcp_fragm{'\n'};
    TlsServer tls_fragm{'\n'};

    // Optionally you can give a maximum length of messages (incoming and sent) for fragmentation mode
    // When using the example above, the maximum message length is set to the maximum a string can handle on your system
    TcpServer tcp_fragm_short{'\n', 100};
    TlsServer tls_fragm_short{'\n', 100};

    // Forwarding mode
    TcpServer tcp_fwd;
    TlsServer tls_fwd;
    ```

### Passing worker function

Passing worker functions might be a bit tricky depending on the definition functions definition.\
The following examples only show passing the worker for established connections. Other workers can be passed similarly.\
The following cases can be handled as shown:

1. Standalone function:

    The easiest way is using a standalone function that is not a part of any class.

    ```cpp
    void standalone(const int clientId)
    {
        // Some code
    }

    TcpServer tcpServer;
    tcpServer.setWorkOnEstablished(&standalone);
    ```

1. Member function of this:

    A worker function could also be defined as a class method. If the TCP/TLS server shall be created within the same class that holds the worker function (e.g. in initializer list), this can be done as follows:

    ```cpp
    class ExampleClass
    {
    public:
        ExampleClass()
        {
            tcpServer.setWorkOnEstablished(::std::bind(&ExampleClass::classMember, this, ::std::placeholders::_1));
        }
        virtual ~ExampleClass() {}

    private:
        // TCP server as class member
        TcpServer tcpServer{};

        void classMember(const int clientId)
        {
            // Some code
        }
    };
    ```

    The **bind** function is used to get the function reference to a method from an object, in this case ```this```. For each attribute of the passed function, a placeholder with increasing number must be passed.

1. Member function of foreign class:

    Passing a member function from a foreign class to TCP/TLS server can be done similarly to above example.

    ```cpp
    class ExampleClass
    {
    public:
        ExampleClass() {}
        virtual ~ExampleClass() {}

    private:
        void classMember(const int clientId)
        {
            // Some code
        }
    };

    // Create object
    ExampleClass exampleClass;

    // TCP server outside from class
    TcpServer tcpServer;
    tcpServer.setWorkOnEstablished(::std::bind(&ExampleClass::classMember, exampleClass, ::std::placeholders::_1));
    ```

1. Lambda

    A worker function could also be defined directly using a lambda.

    ```cpp
    TcpServer tcpServer;
    tcpServer.setWorkOnEstablished([](const int clientId)
    {
        // Some code
    });
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

### [Pipe error if client sends immediately after exiting start](https://github.com/nilshenrich/NetworkListener/issues/21)

When a client sends a message to the listener immediately after the NetworkClient::start() method returned, the listener program throws a pipe error.

Waiting for a short time after connecting to server will fix it on client side.
