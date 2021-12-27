# For debugging please use the command "cmake -DCMAKE_BUILD_TYPE=Debug .."

# Minimum required version of CMake
cmake_minimum_required(VERSION 3.10)

# Project name
project(NetworkListener VERSION 1.0)

# Compile example and all cpp files in src directory
file(GLOB sourcefiles
    "src/*.cpp"
    "example.cpp"
    )

# The deault build type is Release
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# Source files to be compiled
add_executable(NetworkListener ${sourcefiles})

# Set include path to search for headers
include_directories(include)

# Set compiler and linker flags
target_compile_options(NetworkListener PRIVATE -fexceptions
    $<$<CONFIG:DEBUG>: -DDEVELOP -Wall -g -Og>
    $<$<CONFIG:RELEASE>: -O3>)
target_link_options(NetworkListener PRIVATE $<$<CONFIG:RELEASE>: -s>)
target_link_libraries(NetworkListener -lcrypto -lcrypt -lssl -pthread)

# Use C++17 standard
set_target_properties(NetworkListener PROPERTIES
    CXX_STANDARD 17
    CMAKE_CXX_STANDARD_REQUIRED True)
