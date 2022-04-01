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

#include "NetworkListener/TcpServer.h"
#include "NetworkListener/TlsServer.h"

using namespace std;
using namespace networking;

// Example class that derives from TcpServer and TlsServer
class ExampleServer : private TcpServer, private TlsServer
{
public:
    // Constructor and destructor
    ExampleServer() : TcpServer{'\x00'}, TlsServer{'\x00'} {}
    virtual ~ExampleServer() {}

    // Start TCP and TLS server
    int start()
    {
        // Start TCP server
        int start_tcp{TcpServer::start(8081)};

        // Start TLS server
        int start_tls{TlsServer::start(8082, "../keys/ca/ca_cert.pem", "../keys/server/server_cert.pem", "../keys/server/server_key.pem")};

        // Return code (2 bytes): High byte: TLS, low byte: TCP
        return (start_tls << 8) | start_tcp;
    }

    // Stop TCP and TLS server
    void stop()
    {
        // Stop TCP server
        TcpServer::stop();

        // Stop TLS server
        TlsServer::stop();

        return;
    }

private:
    // Override abstract methods
    void workOnMessage_TcpServer(const int tcpClientId, const std::string tcpMsgFromClient)
    {
        cout << "Message from TCP client " << tcpClientId << ": " << tcpMsgFromClient << endl;
        TcpServer::sendMsg(tcpClientId, "Send back: " + tcpMsgFromClient);
        return;
    }

    void workOnClosed_TcpServer(const int tcpClientId)
    {
        cout << "TCP Client " << tcpClientId << " closed connection." << endl;
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
    int start{server.start()};
    if (start)
    {
        cerr << "Error when starting server: " << start << endl;
        return start;
    }

    cout << "Server started." << endl;

    // Wait for 10 seconds
    this_thread::sleep_for(10s);

    // Stop server
    server.stop();

    cout << "Server stopped." << endl;

    return 0;
}
