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
       * @param os
       */
      TcpServer(std::function<std::ostream *(int)> os = nullptr);

      /**
       * @brief Constructor for fragmented messages
       *
       * @param delimiter
       * @param messageMaxLen
       */
      TcpServer(char delimiter, size_t messageMaxLen = std::numeric_limits<size_t>::max() - 1);

      /**
       * @brief Destructor
       */
      virtual ~TcpServer();

      /**
       * @brief Do some stuff when a new message is received from a specific client (Identified by its TCP ID).
       * This method must be implemented in derived classes.
       *
       * @param tcpClientId
       * @param tcpMsgFromClient
       */
      virtual void workOnMessage_TcpServer(const int tcpClientId, const std::string tcpMsgFromClient) = 0;

      /**
       * @brief Do some stuff when a connection to a specific client (Identified by its TCP ID) is closed.
       * This method must be implemented in derived classes.
       *
       * @param tcpClientId
       */
      virtual void workOnClosed_TcpServer(const int tcpClientId) = 0;

   private:
      /**
       * @brief Initialize the server (Do nothing. Just return 0).
       *
       * @return int
       */
      int init(const char *const,
               const char *const,
               const char *const) override final;

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

      /**
       * @brief Just call specific handler method for TCP server (workOnMessage_TcpServer).
       *
       * @param clientId
       * @param msg
       */
      void workOnMessage(const int clientId, const std::string msg) override final;

      /**
       * @brief Just call specific handler method for TCP server (workOnClosed_TcpServer).
       *
       * @param clientId
       */
      void workOnClosed(const int clientId) override final;

      // Disallow copy
      TcpServer(const TcpServer &) = delete;
      TcpServer &operator=(const TcpServer &) = delete;
   };
}

#endif // TCPSERVER_H
