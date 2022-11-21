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

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <thread>
#include <mutex>
#include <cstring>
#include <exception>
#include <limits>
#include <atomic>
#include <memory>
#include <functional>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "NetworkingDefines.h"

namespace networking
{
    /**
     * @brief Exception class for the NetworkListener class.
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
        const std::string msg;

        // Delete default constructor
        NetworkListener_error() = delete;

        // Disallow copy
        NetworkListener_error(const NetworkListener_error &) = delete;
        NetworkListener_error &operator=(const NetworkListener_error &) = delete;
    };

    /**
     * @brief Class to manage running flag in threads.
     *
     */
    using RunningFlag = std::atomic_bool;
    class NetworkListener_running_manager
    {
    public:
        NetworkListener_running_manager(RunningFlag &flag) : flag{flag} {}
        virtual ~NetworkListener_running_manager()
        {
            flag = false;
        }

    private:
        RunningFlag &flag;

        // Delete default constructor
        NetworkListener_running_manager() = delete;

        // Disallow copy
        NetworkListener_running_manager(const NetworkListener_running_manager &) = delete;
        NetworkListener_running_manager &operator=(const NetworkListener_running_manager &) = delete;
    };

    /**
     * @brief Template class for the NetworkListener class.
     * A usable server class must be derived from this class with specific socket type (int for unencrypted TCP, SSL for TLS).
     *
     * @param SocketType
     * @param SocketDeleter
     */
    template <class SocketType, class SocketDeleter = std::default_delete<SocketType>>
    class NetworkListener
    {
    public:
        /**
         * @brief Constructor for continuous stream forwarding
         *
         * @param os            Function to create forwarding stream based on client ID
         * @param workOnClosed  Working function on closed connection
         */
        NetworkListener(std::function<std::ostream *(int)> os,
                        std::function<void(const int)> workOnClosed) : generateNewForwardStream{os},
                                                                       workOnMessage{nullptr},
                                                                       workOnClosed{workOnClosed},
                                                                       DELIMITER_FOR_FRAGMENTATION{0},
                                                                       MAXIMUM_MESSAGE_LENGTH_FOR_FRAGMENTATION{0},
                                                                       MESSAGE_FRAGMENTATION_ENABLED{false} {}

        /**
         * @brief Constructor for fragmented messages
         *
         * @param delimiter     Character to split messages on
         * @param messageMaxLen Maximum message length
         * @param workOnMessage Working function on incoming message
         * @param workOnClosed  Working function on closed connection
         */
        NetworkListener(char delimiter, size_t messageMaxLen,
                        std::function<void(const int, const std::string)> workOnMessage,
                        std::function<void(const int)> workOnClosed) : generateNewForwardStream{nullptr},
                                                                       workOnMessage{workOnMessage},
                                                                       workOnClosed{workOnClosed},
                                                                       DELIMITER_FOR_FRAGMENTATION{delimiter},
                                                                       MAXIMUM_MESSAGE_LENGTH_FOR_FRAGMENTATION{messageMaxLen},
                                                                       MESSAGE_FRAGMENTATION_ENABLED{true} {}

        /**
         * @brief Destructor
         */
        virtual ~NetworkListener() {}

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
        int start(const int port,
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
        std::string getClientIp(const int clientId) const;

        /**
         * @brief Return if listener is running
         *
         * @return bool (true if running, false if not)
         */
        bool isRunning() const;

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
         * @brief Initializes a new connection just after accepting it on unencrypted TCP level.
         * The returned socket is used to communicate with the client.
         * This method is abstract and must be implemented by derived classes.
         *
         * @param clientId
         * @return SocketType*
         */
        virtual SocketType *connectionInit(const int clientId) = 0;

        /**
         * @brief Deinitialize a connection just before closing it.
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
        void listenerReceive(const int clientId, RunningFlag *const recRunning_p);

        // Socket address for the listener
        struct sockaddr_in socketAddress
        {
        };

        // TCP socket for the listener to accept new connections
        int tcpSocket{0};

        // Thread to accept new connections
        std::thread accHandler{};

        // All receiving threads (One per connected client) and their running status
        std::map<int, std::thread> recHandlers{};
        std::map<int, std::unique_ptr<RunningFlag>> recHandlersRunning{};

        // Flag to indicate if the listener is running
        RunningFlag running{false};

        // Pointer to a function that returns an out stream to forward incoming data to
        std::function<std::ostream *(int)> generateNewForwardStream;
        std::map<int, std::unique_ptr<std::ostream>> forwardStreams;

        // Pointer to worker functions on incoming message (for fragmentation mode only) or closed connection
        std::function<void(const int, const std::string)> workOnMessage;
        std::function<void(const int)> workOnClosed;

        // Delimiter for the message framing (incoming and outgoing) (default is '\n')
        const char DELIMITER_FOR_FRAGMENTATION;

        // Maximum message length (incoming and outgoing) (default is 2³² - 2 = 4294967294)
        const size_t MAXIMUM_MESSAGE_LENGTH_FOR_FRAGMENTATION;

        // Flag if messages shall be fragmented
        const bool MESSAGE_FRAGMENTATION_ENABLED;

        // Disallow copy
        NetworkListener() = delete;
        NetworkListener(const NetworkListener &) = delete;
        NetworkListener &operator=(const NetworkListener &) = delete;
    };

    // ============================== Implementation of non-abstract methods. ==============================
    // ====================== Must be in header file because of the template class. =======================

#include "NetworkListener.tpp"
}

#endif // NETWORKLISTENER_H
