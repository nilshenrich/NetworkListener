/**
 * @file NetworkListener.h
 * @author Nils Henrich
 * @brief Base framework for all classes that build a network server based on TCP.
 * This class contains no functionality, but serves as a base framework for the creation of stable servers based on TCP.
 * When compiling with the -DDEBUG flag, the class will print out all received messages to the console.
 * @version 1.0
 * @date 2021-12-27
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef NETWORKLISTENER_H
#define NETWORKLISTENER_H

#ifdef DEVELOP
#include <iostream>
#endif // DEVELOP

#include <string>
#include <map>
#include <thread>
#include <mutex>
#include <cstring>
#include <exception>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "NetworkingDefines.h"

namespace networking
{
    /**
     * @brief Expeption class for the NetworkListener class.
     */
    class NetworkListener_error : public std::exception
    {
    public:
        NetworkListener_error(std::string msg = "unexpected networking error") : msg{msg} {}
        virtual ~NetworkListener_error() {}

        const char *what()
        {
            return msg.c_str();
        }

    private:
        std::string msg;

        // Delete default constructor
        NetworkListener_error() = delete;

        // Disallow copy
        NetworkListener_error(const NetworkListener_error &) = delete;
        NetworkListener_error &operator=(const NetworkListener_error &) = delete;
    };

    /**
     * @brief Template class for the NetworkListener class.
     * A usable server class must be derived from this class with specific socket type (int for unencrypted TCP, SSL for TLS).
     * 
     * @tparam SocketType 
     * @tparam SocketDeleter 
     */
    template <class SocketType, class SocketDeleter = std::default_delete<SocketType>>
    class NetworkListener
    {
    public:
        NetworkListener() {}
        virtual ~NetworkListener()
        {
            stop();
        }

        /**
         * @brief Starts the listener.
         * If listener was started successfully (return value NETWORKLISTENER_START_OK), the listener can accept new connections and send and receive data.
         * If encryption should be used, the server must be started with the correct path to the CA certificate and the correct path to the certificate and key file.
         * 
         * @param port 
         * @param pathToCaCert 
         * @param pathToCert 
         * @param pathToPrivKey 
         * @return int (NETWORKLISTENER_START_OK if successful, see NetworkListenerDefines.h for other return values)
         */
        virtual int start(const int port,
                          const char *const pathToCaCert = nullptr,
                          const char *const pathToCert = nullptr,
                          const char *const pathToPrivKey = nullptr);

        /**
         * @brief Stops the listener.
         * When stopping the listener, all active connections are closed.
         */
        void stop();

        /**
         * @brief Sends a message to a specific client (Identified by its TCP ID).
         * 
         * @param clientId 
         * @param msg 
         * @return bool (true if successful, false if not)
         */
        bool sendMsg(const int clientId, const std::string &msg);

        /**
         * @brief Get the IP address of a specific connected client (Identified by its TCP ID).
         * 
         * @param clientId 
         * @return std::string 
         */
        std::string getClientIp(const int clientId);

    protected:
        /**
         * @brief Initializes the listener just before starting it.
         * This method is abstract and must be implemented by derived classes.
         * 
         * @param pathToCaCert 
         * @param pathToCert 
         * @param pathToPrivKey 
         * @return int 
         */
        virtual int init(const char *const pathToCaCert,
                         const char *const pathToCert,
                         const char *const pathToPrivKey) = 0;

        /**
         * @brief Deinitializes the listener just after stopping it.
         * This method is abstract and must be implemented by derived classes.
         */
        virtual void deinit() = 0;

        /**
         * @brief Initializes a new connetion just after accepting it on unencrypted TCP level.
         * The returned socket is used to communicate with the client.
         * This method is abstract and must be implemented by derived classes.
         * 
         * @param clientId 
         * @return SocketType* 
         */
        virtual SocketType *connectionInit(const int clientId) = 0;

        /**
         * @brief Deinitializes a connection just before closing it.
         * This method is abstract and must be implemented by derived classes.
         * 
         * @param socket 
         */
        virtual void connectionDeinit(SocketType *socket) = 0;

        /**
         * @brief Read raw received data from a specific client (Identified by its TCP ID).
         * This method is expected to return the read raw data as a string with blocking read (Empty string means failure).
         * This method is abstract and must be implemented by derived classes.
         * 
         * @param socket 
         * @return std::string 
         */
        virtual std::string readMsg(SocketType *socket) = 0;

        /**
         * @brief Send raw data to a specific client (Identified by its TCP ID).
         * This method is called by the sendMsg method.
         * This method is abstract and must be implemented by derived classes.
         * 
         * @param clientId 
         * @param msg 
         * @return bool 
         */
        virtual bool writeMsg(const int clientId, const std::string &msg) = 0;

        /**
         * @brief Do some stuff when a new message is received from a specific client (Identified by its TCP ID).
         * This method is called automatically as soon as a new message is received.
         * This method is abstract and must be implemented by derived classes.
         * 
         * @param clientId 
         * @param msg 
         */
        virtual void workOnMessage(const int clientId, const std::string msg) = 0;

        /**
         * @brief Do some stuff when a connection to a specific client (Identified by its TCP ID) is closed.
         * This method is called automatically as soon as a connection is closed or broken.
         * This method is abstract and must be implemented by derived classes.
         * 
         * @param clientId 
         */
        virtual void workOnClosed(const int clientId) = 0;

        // Map to store all active connections with their identifying TCP ID
        std::map<int, std::unique_ptr<SocketType, SocketDeleter>> activeConnections{};

        // Mutex to protect the activeConnections map
        std::mutex activeConnections_m{};

        // Maximum TCP packet size
        const static int MAXIMUM_RECEIVE_PACKAGE_SIZE{16384};

    private:
        /**
         * @brief Accept new connections.
         * This method runs infinitely in a separate thread while the listener is running.
         */
        void listenerAccept();

        /**
         * @brief Read incoming data from a specific connected client (Identified by its TCP ID).
         * This method runs infinitely in a separate thread while the specific client is connected.
         * 
         * @param clientId 
         */
        void listenerReceive(const int clientId);

        // Flag to indicate if the listener is running
        bool running{false};

        // Socket address for the listener
        struct sockaddr_in socketAddress
        {
        };

        // TCP socket for the listener to accept new connections
        int tcpSocket{0};

        // Thread to accept new connections
        std::thread accHandler{};

        // Disallow copy
        NetworkListener(const NetworkListener &) = delete;
        NetworkListener &operator=(const NetworkListener &) = delete;
    };

    // ============================== Implementation of non-abstract methods. ==============================
    // ====================== Must be in header file beccause of the template class. =======================

    template <class SocketType, class SocketDeleter>
    int NetworkListener<SocketType, SocketDeleter>::start(
        const int port,
        const char *const pathToCaCert,
        const char *const pathToCert,
        const char *const pathToPrivKey)
    {
        using namespace std;

        // If the listener is already running, return error
        if (running)
        {
#ifdef DEVELOP
            cerr << typeid(this).name() << "::" << __func__ << ": Listener already running" << endl;
#endif // DEVELOP

            return -1;
        }

        // Check if the port number is valid
        if (1 > port || 65535 < port)
        {
#ifdef DEVELOP
            cerr << typeid(this).name() << "::" << __func__ << ": The port " << port << " couldn't be used" << endl;
#endif // DEVELOP

            return NETWORKLISTENER_ERROR_START_WRONG_PORT;
        }

        // Initialize the listener and return error if it fails
        int initCode{init(pathToCaCert, pathToCert, pathToPrivKey)};
        if (initCode)
            return initCode;

        // Create the TCP socket for the listener to accept new connections.
        // Return error if it fails
        tcpSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (-1 == tcpSocket)
        {
#ifdef DEVELOP
            cerr << typeid(this).name() << "::" << __func__ << ": Error when creating TCP socket to listen on" << endl;
#endif // DEVELOP

            // Stop the listener
            stop();

            return NETWORKLISTENER_ERROR_START_CREATE_SOCKET;
        }

        // Set options on the TCP socket for the listener to accept new connections.
        // (Reuse address)
        // Return error if it fails
        int opt{0};
        if (setsockopt(tcpSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
        {
#ifdef DEVELOP
            cerr << typeid(this).name() << "::" << __func__ << ": Error when setting TCP socket options" << endl;
#endif // DEVELOP

            // Stop the listener
            stop();

            return NETWORKLISTENER_ERROR_START_SET_SOCKET_OPT;
        }

        // Initialize the socket address for the listener.
        memset(&socketAddress, 0, sizeof(socketAddress));
        socketAddress.sin_family = AF_INET;
        socketAddress.sin_addr.s_addr = INADDR_ANY;
        socketAddress.sin_port = htons(port);

        // Bind the TCP socket for the listener to accept new connections to the socket address.
        // Return error if it fails
        if (bind(tcpSocket, (struct sockaddr *)&socketAddress, sizeof(socketAddress)))
        {
#ifdef DEVELOP
            cerr << typeid(this).name() << "::" << __func__ << ": Error when binding listener to port " << port << endl;
#endif // DEVELOP

            // Stop the listener
            stop();

            return NETWORKLISTENER_ERROR_START_BIND_PORT;
        }

        // Start listening on the TCP socket for the listener to accept new connections.
        if (listen(tcpSocket, SOMAXCONN))
        {
#ifdef DEVELOP
            cerr << typeid(this).name() << "::" << __func__ << ": Error when starting listening" << endl;
#endif // DEVELOP

            // Stop the listener
            stop();

            return NETWORKLISTENER_ERROR_START_LISTENER;
        }

        // Listener is now running
        running = true;

        // Start the thread to accept new connections
        if (accHandler.joinable())
            throw NetworkListener_error("Start listener thread failed: Thread is already running"s);
        accHandler = thread{&NetworkListener::listenerAccept, this};

        return initCode;
    }

    template <class SocketType, class SocketDeleter>
    void NetworkListener<SocketType, SocketDeleter>::stop()
    {
        using namespace std;

        // Stop the listener
        running = false;

        // Block listening TCP socket to abort all reads
        if (shutdown(tcpSocket, SHUT_RDWR))
            return;

        // Wait for the accept thread to finish
        if (accHandler.joinable())
            accHandler.join();

        // Close listening TCP socket
        close(tcpSocket);

        // Deinitialize the listener
        deinit();

        return;
    }

    template <class SocketType, class SocketDeleter>
    bool NetworkListener<SocketType, SocketDeleter>::sendMsg(const int clientId, const std::string &msg)
    {
        using namespace std;

        // Extend message with start and end characters and send it
        return writeMsg(clientId, string{NETWORKLISTENER_CHAR_TRANSFER_START} + msg + string{NETWORKLISTENER_CHAR_TRANSFER_END});
    }

    template <class SocketType, class SocketDeleter>
    std::string NetworkListener<SocketType, SocketDeleter>::getClientIp(const int clientId)
    {
        using namespace std;

        struct sockaddr_in addr;
        socklen_t addrSize = sizeof(struct sockaddr_in);
        if (getpeername(clientId, (struct sockaddr *)&addr, &addrSize))
        {
#ifdef DEVELOP
            cerr << typeid(this).name() << "::" << __func__ << ": Error reading client " << clientId << "'s IP address" << endl;
#endif // DEVELOP

            return "Failed Read!";
        }

        // Convert the IP address to a string and return it
        return string{inet_ntoa(addr.sin_addr)};
    }

    template <class SocketType, class SocketDeleter>
    void NetworkListener<SocketType, SocketDeleter>::listenerAccept()
    {
        using namespace std;

        // Get the size of the socket address for the listener (important for connection establishment)
        socklen_t socketAddress_len{sizeof(socketAddress)};

        // Accept new connections while the listener is running
        while (running)
        {
            // Wait for a new connection to accept
            const int newConnection{accept(tcpSocket, (struct sockaddr *)&socketAddress, &socketAddress_len)};

            // If new accepted connection ID is -1, the accept failed
            // In this case, continue with accepting the new connections
            if (-1 == newConnection)
                continue;

#ifdef DEVELOP
            cout << typeid(this).name() << "::" << __func__ << ": New client connected: " << newConnection << endl;
#endif // DEVELOP

            // When a new connection is established (Unencrypted so far), the incoming messages of this connection should be read in a new process
            thread{&NetworkListener::listenerReceive, this, newConnection}.detach();
        }

        // Close all active connections
        {
            lock_guard<mutex> lck{activeConnections_m};
            for (const auto &it : activeConnections)
            {
                shutdown(it.first, SHUT_RDWR);

#ifdef DEVELOP
                cout << typeid(this).name() << "::" << __func__ << ": Closed connection to cliet " << it.first << endl;
#endif // DEVELOP
            }
        }

        // Wait for all receive processes to finish
        // Until the map of active connections is empty
        while (!activeConnections.empty())
            this_thread::sleep_for(100ms);

        return;
    }

    template <class SocketType, class SocketDeleter>
    void NetworkListener<SocketType, SocketDeleter>::listenerReceive(const int clientId)
    {
        using namespace std;

        // Initialize the (so far uncrypted) connection
        SocketType *connection_p{connectionInit(clientId)};
        if (!connection_p)
            return;

        // Add connection to activ connections
        {
            lock_guard<mutex> lck{activeConnections_m};
            activeConnections[clientId] = unique_ptr<SocketType, SocketDeleter>{connection_p};
        }

        // Read incoming messages from this connection as long as the connection is active
        string buffer;
        while (1)
        {
            // Wait for new incoming message (iplemented in derived classes)
            // If message is empty string, the connection is broken
            string msg{readMsg(connection_p)};
            if (msg.empty())
            {
#ifdef DEVELOP
                cout << typeid(this).name() << "::" << __func__ << ": Connection to client " << clientId << " broken" << endl;
#endif // DEVELOP

                {
                    lock_guard<mutex> lck{activeConnections_m};

                    // Deinitialize the connection
                    connectionDeinit(connection_p);

                    // Block the connection from being used anymore
                    shutdown(clientId, SHUT_RDWR);

                    // Remove connection from active connections
                    activeConnections.erase(clientId);
                }

                // Run code to handle the closed connection
                workOnClosed(clientId);

                // Close the connection
                close(clientId);

                // End receiving process of this connection
                return;
            }

            // The message is between STX and ETX in the stream
            // Extract the message
            for (char c : msg)
            {
                switch (c)
                {
                // Start of message -> empty buffer
                case NETWORKLISTENER_CHAR_TRANSFER_START:
                    buffer.clear();
                    break;

                // End of message -> process the message (Buffer gets cleared by move)
                case NETWORKLISTENER_CHAR_TRANSFER_END:
#ifdef DEVELOP
                    cout << typeid(this).name() << "::" << __func__ << ": Message from client " << clientId << ": " << msg << endl;
#endif // DEVELOP
                    thread{&NetworkListener::workOnMessage, this, clientId, move(buffer)}.detach();
                    break;

                // Midder of message -> store character in buffer
                default:
                    buffer.push_back(c);
                    break;
                }
            }
        }
    }
}

#endif // NETWORKLISTENER_H
