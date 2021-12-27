/**
 * @file example.cpp
 * @author Nils Henrich
 * @brief Example how to use the networking library
 * @version 1.0
 * @date 2021-12-27
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include <iostream>

#include "TcpServer.h"
#include "TlsServer.h"

using namespace std;
using namespace networking;

// Example class that derives from TcpServer and TlsServer
class ExampleServer : private TcpServer, private TlsServer
{
public:
    // Constructor and destructor
    ExampleServer() {}
    virtual ~ExampleServer() {}

private:
    // Override abstract methods
    void workOnMessage_TcpServer(const int tlsClientId, const std::string tlsMsgFromClient)
    {
        cout << "Message from TCP client " << tlsClientId << ": " << tlsMsgFromClient << endl;
    }

    void workOnClosed_TcpServer(const int tlsClientId)
    {
        cout << "TCP Client " << tlsClientId << " closed connection." << endl;
    }

    void workOnMessage_TlsServer(const int tlsClientId, const std::string tlsMsgFromClient)
    {
        cout << "Message from TLS client " << tlsClientId << ": " << tlsMsgFromClient << endl;
    }

    void workOnClosed_TlsServer(const int tlsClientId)
    {
        cout << "TLS Client " << tlsClientId << " closed connection." << endl;
    }
};

int main()
{
    ExampleServer server;

    return 0;
}
