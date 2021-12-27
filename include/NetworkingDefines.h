/**
 * @file NetworkingDefines.h
 * @author Nils Henrich
 * @brief Basic definitions for the network listener.
 * @version 1.0
 * @date 2021-12-27
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef TLSSERVERDEFINES_H_INCLUDED
#define TLSSERVERDEFINES_H_INCLUDED

namespace networking
{
    enum : int
    {
        NETWORKING_START_OK = 0,                     // Listener started successfully
        NETWORKING_ERROR_START_WRONG_PORT = 10,      // Listener could not start because of wrong port number
        NETWORKING_ERROR_START_SET_CONTEXT = 20,     // Listener could not start because of SSL context error
        NETWORKING_ERROR_START_WRONG_CA_PATH = 30,   // Listener could not start because of wrong path to CA cert file
        NETWORKING_ERROR_START_WRONG_CERT_PATH = 31, // Listener could not start because of wrong path to certificate file
        NETWORKING_ERROR_START_WRONG_KEY_PATH = 32,  // Listener could not start because of wrong path to key file
        NETWORKING_ERROR_START_WRONG_KEY = 33,       // Listener could not start because of non matching key and certificate
        NETWORKING_ERROR_START_CREATE_SOCKET = 40,   // Listener could not start because of TCP socket creation error
        NETWORKING_ERROR_START_SET_SOCKET_OPT = 41,  // Listener could not start because of TCP socket option error
        NETWORKING_ERROR_START_BIND_PORT = 42,       // Listener could not start because of TCP socket bind error
        NETWORKING_ERROR_START_LISTENER = 43         // Listener could not start because of TCP socket listen error
    };

    enum : char
    {
        NETWORKING_CHAR_TRANSFER_START = '\x02',
        NETWORKING_CHAR_TRANSFER_END = '\x03'
    };
}

#endif // TLSSERVERDEFINES_H_INCLUDED
