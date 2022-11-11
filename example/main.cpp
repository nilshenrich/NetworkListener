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
#include <fstream>
#include <chrono>

#include "NetworkListener/TcpServer.h"
#include "NetworkListener/TlsServer.h"

using namespace std;
using namespace networking;

// File streams for continuous stream forwarding
ofstream StreamForward_tcp{"output_tcp.txt"};
ofstream StreamForward_tls{"output_tls.txt"};

// Example class for fragmented message transfer that derives from TcpServer and TlsServer
class ExampleServer_fragmented : private TcpServer, private TlsServer
{
public:
    // Constructor and destructor
    ExampleServer_fragmented() : TcpServer{'\x00'}, TlsServer{'\x00'} {}
    virtual ~ExampleServer_fragmented() {}

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

// Example class for continuous message transfer that derives from TcpServer and TlsServer
class ExampleServer_continuous : private TcpServer, private TlsServer
{
public:
    // Constructor and destructor
    ExampleServer_continuous() : TcpServer{StreamForward_tcp}, TlsServer{StreamForward_tls} {}
    virtual ~ExampleServer_continuous() {}

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
        cerr << "Work on TCP message should never be executed" << endl;
        return;
    }

    void workOnClosed_TcpServer(const int tcpClientId)
    {
        cerr << "Work on TCP close should never be executed" << endl;
        return;
    }

    void workOnMessage_TlsServer(const int tlsClientId, const std::string tlsMsgFromClient)
    {
        cerr << "Work on TLS message should never be executed" << endl;
        return;
    }

    void workOnClosed_TlsServer(const int tlsClientId)
    {
        cerr << "Work on TLS close should never be executed" << endl;
        return;
    }
};

int main()
{
    // Create servers
    ExampleServer_fragmented server_fragmented;
    ExampleServer_continuous server_continuous;

    // Start servers
    int start{server_fragmented.start() & server_continuous.start()};
    if (start)
    {
        cerr << "Error when starting servers: " << start << endl;
        return start;
    }

    cout << "Servers started." << endl;

    // Wait for 10 seconds
    this_thread::sleep_for(10s);

    // Stop servers
    server_fragmented.stop();
    server_continuous.stop();

    cout << "Servers stopped." << endl;

    StreamForward_tcp.flush();
    StreamForward_tls.flush();

    return 0;
}
