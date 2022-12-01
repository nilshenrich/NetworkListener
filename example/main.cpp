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
#include <thread>
#include <memory>

#include "NetworkListener/TcpServer.h"
#include "NetworkListener/TlsServer.h"

using namespace std;
using namespace networking;

unique_ptr<TcpServer> tcpServer;
unique_ptr<TlsServer> tlsServer;

// Global functions
void tcp_fragmented_workOnMessage(int id, string msg) { cout << "Message from TCP client " << id << ": " << msg << endl; }
void tcp_fragmented_workOnEstablished(int id) { tcpServer->sendMsg(id, "Hello TCP client "s + to_string(id) + "! - fragmented mode"s); }
void tcp_forwarding_workOnEstablished(int id) { tcpServer->sendMsg(id, "Hello TCP client "s + to_string(id) + "! - forwarding mode"s); }
ofstream *tcp_forwarding_messageStream(int id) { return new ofstream{"MessageStream_TCP_Client"s + to_string(id)}; }
void tcp_workOnClosed(int id) { cout << "Connection to TCP client " << id << " closed" << endl; }

void tls_fragmented_workOnMessage(int id, string msg) { cout << "Message from TLS client " << id << ": " << msg << endl; }
void tls_fragmented_workOnEstablished(int id) { tlsServer->sendMsg(id, "Hello TLS client "s + to_string(id) + "! - fragmented mode"s); }
void tls_forwarding_workOnEstablished(int id) { tlsServer->sendMsg(id, "Hello TLS client "s + to_string(id) + "! - forwarding mode"s); }
ofstream *tls_forwarding_messageStream(int id) { return new ofstream{"MessageStream_TLS_Client"s + to_string(id)}; }
void tls_workOnClosed(int id) { cout << "Connection to TLS client " << id << " closed" << endl; }

int main()
{
    // User decision
    while (true)
    {
        cout << "What mode shall be used?" << endl
             << "    c: Continuous stream" << endl
             << "    f: Fragmented messages" << endl
             << "    other key: Exit program" << endl;
        char decision;
        cin >> decision;
        switch (decision)
        {
        case 'c':
        case 'C':
        {
            tcpServer.reset(new TcpServer{&tcp_forwarding_workOnEstablished, &tcp_workOnClosed, &tcp_forwarding_messageStream});
            tlsServer.reset(new TlsServer{&tls_forwarding_workOnEstablished, &tls_workOnClosed, &tls_forwarding_messageStream});
            tcpServer->start(8081);
            tlsServer->start(8082, "../keys/ca/ca_cert.pem", "../keys/server/server_cert.pem", "../keys/server/server_key.pem");
            this_thread::sleep_for(10s);
            break;
        }

        case 'f':
        case 'F':
        {
            tcpServer.reset(new TcpServer{'\n', &tcp_fragmented_workOnMessage, &tcp_fragmented_workOnEstablished, &tcp_workOnClosed});
            tlsServer.reset(new TlsServer{'\n', &tls_fragmented_workOnMessage, &tls_fragmented_workOnEstablished, &tls_workOnClosed});
            tcpServer->start(8081);
            tlsServer->start(8082, "../keys/ca/ca_cert.pem", "../keys/server/server_cert.pem", "../keys/server/server_key.pem");
            this_thread::sleep_for(10s);
            break;
        }

        default:
            return 0;
        }
    }
}
