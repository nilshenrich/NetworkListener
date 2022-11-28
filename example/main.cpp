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

#include "NetworkListener/TcpServer.h"
#include "NetworkListener/TlsServer.h"

using namespace std;
using namespace networking;

// Global functions
void tcp_fragmented_workOnMessage(int id, string msg) { cout << "Message from TCP client " << id << ": " << msg << endl; }
ofstream *tcp_forwarding_messageStream(int id) { return new ofstream{"MessageStream_TCP_Client"s + to_string(id), ios::app}; }
void tcp_workOnClosed(int id) { cout << "Connection to TCP client " << id << " closed" << endl; }
void tls_fragmented_workOnMessage(int id, string msg) { cout << "Message from TLS client " << id << ": " << msg << endl; }
ofstream *tls_forwarding_messageStream(int id) { return new ofstream{"MessageStream_TLS_Client"s + to_string(id), ios::app}; }
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
            TcpServer tcpServer{&tcp_workOnClosed, &tcp_forwarding_messageStream};
            TlsServer tlsServer{&tls_workOnClosed, &tls_forwarding_messageStream};
            tcpServer.start(8081);
            tlsServer.start(8082, "../keys/ca/ca_cert.pem", "../keys/server/server_cert.pem", "../keys/server/server_key.pem");
            this_thread::sleep_for(10s);
            break;
        }

        case 'f':
        case 'F':
        {
            TcpServer tcpServer{'\n', &tcp_fragmented_workOnMessage, &tcp_workOnClosed};
            TlsServer tlsServer{'\n', &tls_fragmented_workOnMessage, &tls_workOnClosed};
            tcpServer.start(8081);
            tlsServer.start(8082, "../keys/ca/ca_cert.pem", "../keys/server/server_cert.pem", "../keys/server/server_key.pem");
            this_thread::sleep_for(10s);
            break;
        }

        default:
            return 0;
        }
    }
}
