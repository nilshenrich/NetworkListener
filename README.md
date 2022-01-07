# NetworkListener

## *(A compatible client library is coming soon)*
---

# Table of contents

1. [General explanation](#general-explanation)
1. [Installation](#installation)
1. [Example](#example)
1. [System requirements](#system-requirements)

# General explanation

This project contains installable C++ libraries for a server (listener) on TCP level. A client can connect to the server and data can be sent in both directions. The number of connected clients at one time is limited to 4096.

This packge contains two libraries: **libnetworkListenerTcp** and **liblibnetworkListenerTcp**.\
As the names say, **libnetworkListenerTcp** creates a simple TCP server with no security. The **liblibnetworkListenerTcp** creates a server on TLS level. This means, an established connection is encrypted with the latest compatible TLS version and a client is forced to authenticate itself.

# Installation

As already mentioned in [General explanation](#general-explanation), this project contains two installable libraries **libnetworkListenerTcp** and **liblibnetworkListenerTcp**. These libraries can be installed this way:

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

1. To install the created libraries and header files to your system, run
    ```console
    sudo make install
    ```

1. (optional) In some systems the library paths need to be updated before **libnetworkListenerTcp** and **liblibnetworkListenerTcp** can be used. So if you get an error like
    ```
    ./example: error while loading shared libraries: libnetworkListenerTcp.so.1: cannot open shared object file: No such file or directory
    ```
    please run
    ```console
    sudo /sbin/ldconfig
    ```

Now the package is reade to use. Please see the example for how to use it.

# Example

This repository contains a small example to show the usage of this package. It creates to listeners, one using unseure TCP, the other using unecrypted and two-way-authenticated TLS (two-way authentication means, the server authenticates itself with a CA-signed certificate ad forces the client to also authenticate itself with his own CA-signed certificate).\
The example program runs for 10 seconds. Within this time range, it can accept new client connections and receive data. Received data will be printed on the screen and sent back to the sending client.

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
