/**
 * @file TcpServer.h
 * @author Nils Henrich
 * @brief TCP server for unencrypted data transfer without authentication.
 * @version 1.0
 * @date 2021-12-27
 *
 * @copyright Copyright (c) 2021
 *
 */

#ifndef TCPSERVER_H
#define TCPSERVER_H

#include "NetworkListener.h"

namespace networking
{
   class TcpServer : public NetworkListener<int>
   {
   public:
      /**
       * @brief Constructor for continuous stream forwarding
       *
       * @param os                        function to create forwarding stream based on client ID
       * @param workOnMessage_TcpServer   working function on incoming message
       * @param workOnClosed_TcpServer    working function on closed connection
       */
      TcpServer(std::function<std::ostream *(int)> os = nullptr,
                std::function<void(const int, const std::string)> workOnMessage_TcpServer = nullptr,
                std::function<void(const int)> workOnClosed_TcpServer = nullptr);
      template <class T>
      TcpServer(std::ostream *(T::*os)(int) = nullptr,
                void (T::*workOnMessage_TcpServer)(const int, const std::string) = nullptr,
                void (T::*workOnClosed_TcpServer)(const int) = nullptr);

      /**
       * @brief Constructor for fragmented messages
       *
       * @param delimiter                 Character to split messages on
       * @param messageMaxLen             maximum message length
       * @param workOnMessage_TcpServer   working function on incoming message
       * @param workOnClosed_TcpServer    working function on closed connection
       */
      TcpServer(char delimiter, size_t messageMaxLen = std::numeric_limits<size_t>::max() - 1,
                std::function<void(const int, const std::string)> workOnMessage_TcpServer = nullptr,
                std::function<void(const int)> workOnClosed_TcpServer = nullptr);
      template <class T>
      TcpServer(char delimiter, size_t messageMaxLen = std::numeric_limits<size_t>::max() - 1,
                void (T::*workOnMessage_TcpServer)(const int, const std::string) = nullptr,
                void (T::*workOnClosed_TcpServer)(const int) = nullptr);

      /**
       * @brief Destructor
       */
      virtual ~TcpServer();

   private:
      /**
       * @brief Initialize the server (Do nothing. Just return 0).
       *
       * @return int
       */
      int init(const char *const = nullptr,
               const char *const = nullptr,
               const char *const = nullptr) override final;

      /**
       * @brief Initialize connection to a specific client (Identified by its TCP ID) (Do nothing. Just return pointer to TCP ID).
       *
       * @param clientId
       * @return int*
       */
      int *connectionInit(const int clientId) override final;

      /**
       * @brief Deinitialize connection to a specific client (Identified by its TCP ID) (Do nothing.).
       *
       * @param socket
       */
      void connectionDeinit(int *socket) override final;

      /**
       * @brief Read data from a specific client (Identified by its TCP ID).
       * This method blocks until data is available.
       * If no data is available, it returns an empty string.
       *
       * @param socket
       * @return std::string
       */
      std::string readMsg(int *socket) override final;

      /**
       * @brief Send raw data to a specific client (Identified by its TCP ID).
       * Send message over unencrypted TCP connection.
       *
       * @param clientId
       * @param msg
       * @return true
       * @return false
       */
      bool writeMsg(const int clientId, const std::string &msg) override final;

      // Disallow copy
      TcpServer(const TcpServer &) = delete;
      TcpServer &operator=(const TcpServer &) = delete;
   };
}

#endif // TCPSERVER_H
