# Create library for this package

# Minimum requried version of CMake
cmake_minimum_required(VERSION 3.10)

# Project name
project(networklistener VERSION 1.0)

# Compile all cpp files in the src directory
file(GLOB sourcefiles "src/*.cpp")

# Build type is Release
set(CMAKE_BUILD_TYPE Release)

# Source files to be compiled
add_library(networklistener SHARED ${sourcefiles})

# Set include path to search for headers
include_directories(include)

# Set compiler flags
target_compile_options(networklistener PUBLIC -fexceptions -O3)

# Use C++17 standard
set_target_properties(networklistener PROPERTIES
    CXX_STANDARD 17
    CMAKE_CXX_STANDARD_REQUIRED True)

# Install rule (copy to install directory)
install(TARGETS networklistener
    LIBRARY DESTINATION lib)
