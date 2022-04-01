/**
 * @file TlsServer.h
 * @author Nils Henrich
 * @brief TLS server for encrypted data transfer with authentication.
 * @version 1.0
 * @date 2021-12-27
 *
 * @copyright Copyright (c) 2021
 *
 */

#ifndef TLSSERVER_H
#define TLSSERVER_H

#include <openssl/ssl.h>

#include "NetworkListener.h"

namespace networking
{
   // Deleter for SSL objects
   struct NetworkListener_SSL_Deleter
   {
      void operator()(SSL *ssl)
      {
         SSL_free(ssl);
         return;
      }
   };

   class TlsServer : public NetworkListener<SSL, NetworkListener_SSL_Deleter>
   {
   public:
      TlsServer(char delimiter = '\n');
      virtual ~TlsServer();

      /**
       * @brief Do some stuff when a new message is received from a specific client (Identified by its TCP ID).
       * This method must be implemented in derived classes.
       *
       * @param tlsClientId
       * @param tlsMsgFromClient
       */
      virtual void workOnMessage_TlsServer(const int tlsClientId, const std::string tlsMsgFromClient) = 0;

      /**
       * @brief Do some stuff when a connection to a specific client (Identified by its TCP ID) is closed.
       * This method must be implemented in derived classes.
       *
       * @param tlsClientId
       */
      virtual void workOnClosed_TlsServer(const int tlsClientId) = 0;

      /**
       * @brief Get specific subject part as string of the certificate of a specific connected client (Identified by its TCP ID).
       *
       * @param clientId
       * @param tlsSocket
       * @param subjPart
       * @return std::string
       */
      std::string getSubjPartFromClientCert(const int clientId, const SSL *tlsSocket, const int subjPart);

   private:
      /**
       * @brief Initialize the server (Setup enryyption settings).
       * Setting: Encryption via TLS, force client authentication, authenticate self with certificate.
       * Checking certificate validity with CA certificate.
       *
       * @param pathToCaCert
       * @param pathToCert
       * @param pathToPrivKey
       * @return int
       */
      int init(const char *const pathToCaCert,
               const char *const pathToCert,
               const char *const pathToPrivKey) override final;

      /**
       * @brief Deinitialize the server (Close all encrypted connections).
       *
       */
      void deinit() override final;

      /**
       * @brief Initialize connection to a specific client (Identified by its TCP ID) (Do TLS handshake).
       *
       * @param clientId
       * @return SSL*
       */
      SSL *connectionInit(const int clientId) override final;

      /**
       * @brief Deinitialize connection to a specific client (Identified by its TCP ID) (Do TLS shutdown).
       *
       * @param socket
       */
      void connectionDeinit(SSL *socket) override final;

      /**
       * @brief Read data from a specific client (Identified by its TCP ID).
       * This method blocks until data is available.
       * If no data is available, it returns an empty string.
       *
       * @param socket
       * @return std::string
       */
      std::string readMsg(SSL *socket) override final;

      /**
       * @brief Send raw data to a specific client (Identified by its TCP ID).
       *
       * @param clientId
       * @param msg
       * @return true
       * @return false
       */
      bool writeMsg(const int clientId, const std::string &msg) override final;

      /**
       * @brief Just call specific handler method for TLS server (workOnMessage_TlsServer).
       *
       * @param clientId
       * @param msg
       */
      void workOnMessage(const int clientId, const std::string msg) override final;

      /**
       * @brief Just call specific handler method for TLS server (workOnClosed_TlsServer).
       *
       * @param clientId
       */
      void workOnClosed(const int clientId) override final;

      // TLS context of the server
      SSL_CTX *serverContext{nullptr};

      // Disallow copy
      TlsServer(const TlsServer &) = delete;
      TlsServer &operator=(const TlsServer &) = delete;
   };
}

#endif // TLSSERVER_H
