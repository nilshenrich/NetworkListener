# Create library for this package

# Minimum requried version of CMake
cmake_minimum_required(VERSION 3.10)

# Project name
project(tlsServer VERSION 1.0 DESCRIPTION "Listener for TLS")
project(tcpServer VERSION 1.0 DESCRIPTION "Listener for TCP")

# Build type is Release
set(CMAKE_BUILD_TYPE Release)

# Source files to be compiled
add_library(tlsServer SHARED "src/TlsServer.cpp")
add_library(tcpServer SHARED "src/TcpServer.cpp")

# Set include path to search for headers
include_directories(include)

# Set compiler flags
target_compile_options(tlsServer PUBLIC -fexceptions -O3)
target_compile_options(tcpServer PUBLIC -fexceptions -O3)

# Use C++17 standard
set_target_properties(tlsServer PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    PUBLIC_HEADER "include/TlsServer.h;include/NetworkListener.h;include/NetworkingDefines.h"
    CXX_STANDARD 17
    CMAKE_CXX_STANDARD_REQUIRED True)
set_target_properties(tcpServer PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    PUBLIC_HEADER "include/TcpServer.h;include/NetworkListener.h;include/NetworkingDefines.h"
    CXX_STANDARD 17
    CMAKE_CXX_STANDARD_REQUIRED True)

# Install rule (copy to install directory)
include(GNUInstallDirs)
install(TARGETS tlsServer tcpServer
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/NetworkListener")
