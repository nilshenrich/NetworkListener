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
#include <chrono>

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

    // Start TCP and TLS server
    void start()
    {
        // Start TCP server
        TcpServer::start(8081);

        // Start TLS server
        TlsServer::start(8082, "keys/ca/ca_cert.pem", "keys/server/server_cert.pem", "keys/server/server_key.pem");

        return;
    }

private:
    // Override abstract methods
    void workOnMessage_TcpServer(const int tlsClientId, const std::string tcpMsgFromClient)
    {
        cout << "Message from TCP client " << tlsClientId << ": " << tcpMsgFromClient << endl;
        TcpServer::sendMsg(tlsClientId, "Send back: " + tcpMsgFromClient);
        return;
    }

    void workOnClosed_TcpServer(const int tlsClientId)
    {
        cout << "TCP Client " << tlsClientId << " closed connection." << endl;
        return;
    }

    void workOnMessage_TlsServer(const int tlsClientId, const std::string tlsMsgFromClient)
    {
        cout << "Message from TLS client " << tlsClientId << ": " << tlsMsgFromClient << endl;
        TlsServer::sendMsg(tlsClientId, "Send back: " + tlsMsgFromClient);
        return;
    }

    void workOnClosed_TlsServer(const int tlsClientId)
    {
        cout << "TLS Client " << tlsClientId << " closed connection." << endl;
        return;
    }
};

int main()
{
    // Create server
    ExampleServer server;

    // Start server
    server.start();

    // Wait for 10 seconds
    this_thread::sleep_for(10s);

    return 0;
}
