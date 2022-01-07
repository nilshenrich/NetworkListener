# NetworkListener

Installable package to set up a server clients can connect to on TCP level and send and receive data asynchronously. The connection can be established with unencrypted pure TCP or with encrypted and two-way-authenticated TLS.

The compatible client can be found [here](https://github.com/nilshenrich/NetworkClient)

# Table of contents

1. [General explanation](#general-explanation)
1. [Installation](#installation)
1. [Example](#example)
1. [System requirements](#system-requirements)

# General explanation

This project contains installable C++ libraries for a server (listener) on TCP level. A client can connect to the server and data can be sent in both directions.

This packge contains two libraries: **libnetworkListenerTcp** and **libnetworkListenerTcp**.\
As the names say, **libnetworkListenerTcp** creates a simple TCP server with no security. The **libnetworkListenerTcp** creates a server on TLS level. This means, an established connection is encrypted with the latest compatible TLS version and a client is forced to authenticate itself.

## Specifications

1. Maximum number of connected clients at the same time: **4096**
1. Maximum message size:  **std::string::max_size() - 2** (2<sup>32</sup> - 2 (4294967294) for most)

# Installation

As already mentioned in [General explanation](#general-explanation), this project contains two installable libraries **libnetworkListenerTcp** and **libnetworkListenerTcp**. These libraries can be installed this way:

1. Install **build-essential** and **cmake**:
    ```console
    sudo apt install build-essential cmake
    ```
    **build-essential** contains compilers and libraries to compile C/C++ code on debian-based systems.\
    **cmake** is used to easily create a Makefile.

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
    *To get information printed on the screen, the package can be built in debug mode (Not recommended when installing libraries) by setting the define: __cmake&#160;&#x2011;DCMAKE_BUILD_TYPE=Debug&#160;..__ (Same when compiling example)*

1. To install the created libraries and header files to your system, run
    ```console
    sudo make install
    ```

1. (optional) In some systems the library paths need to be updated before **libnetworkListenerTcp** and **libnetworkListenerTcp** can be used. So if you get an error like
    ```
    ./example: error while loading shared libraries: libnetworkListenerTcp.so.1: cannot open shared object file: No such file or directory
    ```
    please run
    ```console
    sudo /sbin/ldconfig
    ```

Now the package is reade to use. Please see the example for how to use it.

# Example

This repository contains a small example to show the usage of this package. It creates two listeners, one using unsecure TCP, the other using ecrypted and two-way-authenticated TLS (two-way authentication means, the server authenticates itself with a CA-signed certificate ad forces the client to also authenticate itself with his own CA-signed certificate).\
The example program runs for 10 seconds. Within this time range, it can accept new client connections and receive data. Received data will be printed on the screen and sent back to the sending client.

## Create certificates

Before the encrypted TLS listener can run properly, the needed certificates and private keys need to be created. To do this, just run the bash file
```console
example/CMakeLists.txt
```

## Run example

The example can be compiled the same way as the libraries (Without installing at the end):
```console
cd example
mkdir build
cd build
cmake ..
make
./example
```

# System requirements

The installation process in this project is adapted to debian-based linux distros. But smart guys maybe achieve to make it usable on other sytems (In the end it is just C++ code compilable with C++17 standard or higher).
