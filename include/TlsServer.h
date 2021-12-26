#ifndef TLSSERVER_H
#define TLSSERVER_H

#include <openssl/ssl.h>

#include "NetworkListener.h"

namespace HS_Networking
{
struct SSL_Deleter
{
    void operator()(SSL* ssl)
    {
        SSL_free(ssl);
        return;
    }
};
class TlsServer: public NetworkListener<SSL, SSL_Deleter>
{
public:
    TlsServer();
    virtual ~TlsServer();

    /**
     * Wird aufgerufen, sobald der TLS Server eine neue Nachricht empfängt
     * Diese Methode muss von erbenden Klassen überschrieben werden
     * @param tlsClientId
     * @param tlsMsgFromClient
     */
    virtual void workOnMessage_TlsServer(const int tlsClientId, const std::string tlsMsgFromClient) = 0;

    /**
     * Wird aufgerufen, sobald eine Verbindung des TLS Servers abreißt
     * Diese Methode muss von erbenden Klassen überschrieben werden
     * @param tlsClientId
     */
    virtual void workOnClosed_TlsServer(const int tlsClientId) = 0;

    /**
     * Teil des Subjects aus Zertifikat einer verbundenen Komponente auslesen
     * @param clientId
     * @param subjPart
     * @return string
     */
    std::string getSubjPartFromClientCert(const int clientId, const SSL* tlsSocket, const int subjPart);

private:

    /**
     * Initialisieren des TLS Server
     * @param pathToCaCert
     * @param pathToCert
     * @param pathToPrivKey
     * @return int (0 := OK, ~0 := Fehler)
     */
    int init(const char* const pathToCaCert,
             const char* const pathToCert,
             const char* const pathToPrivKey) override final;

    /**
     * Deinitialisieren des Listeners
     */
    void deinit() override final;

    /**
     * TLS Handshake durchführen
     * @param clientId
     * @return SSL* (nullptr := Fehler -> Empfangen abbrechen, Sonst := Initialisierung erfolgreich -> Nachrichten empfangen)
     */
    SSL* connectionInit(const int clientId) override final;

    /**
     * TLS herunterfahren
     * @param socket
     */
    void connectionDeinit(SSL* socket) override final;

    /**
     * Nächste empfangene Nachricht empfangen.
     * @param socket
     * @return string ("" := Empfangen fehlgeschlagen, sonst := Inhalt der Nachricht)
     */
    std::string readMsg(SSL* socket) override final;

    /**
     * Nachricht an einen TLS Client senden.
     * Der Client muss anhand seiner TCP ID identifiziert werden.
     * @param clientId
     * @param msg
     * @return bool (True := Senden erfolgreich, False := Senden fehlgeschlagen)
     */
    bool writeMsg(const int clientId, const std::string& msg) override final;

    /**
     * Eingehende Nachrichten bearbeiten
     * @param clientId
     * @param msg
     */
    void workOnMessage(const int clientId, const std::string msg) override final;

    /**
     * Abgerissene Verbindungen bearbeiten
     * @param clientId
     */
    void workOnClosed(const int clientId) override final;

    // TLS Context des Servers
    SSL_CTX* serverContext {nullptr};

    // Object kann nicht kopiert werden
    TlsServer(const TlsServer&) = delete;
    TlsServer& operator = (const TlsServer&) = delete;
};
}

#endif // TLSSERVER_H
