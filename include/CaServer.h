#ifndef CASERVER_H
#define CASERVER_H

#include "NetworkListener.h"

namespace HS_Networking
{
class CaServer: public NetworkListener<int>
{
public:
    CaServer();
    virtual ~CaServer();

    /**
     * Wird aufgerufen, sobald der CA Server eine neue Nachricht empfängt.
     * Diese Methode muss von erbenden Klassen überschrieben werden.
     * @param tlsClientId
     * @param tlsMsgFromClient
     */
    virtual void workOnMessage_CaServer(const int tlsClientId, const std::string tlsMsgFromClient) = 0;

    /**
     * Wird aufgerufen, sobald eine Verbindung des CA Servers abreißt
     * Diese Methode muss von erbenden Klassen überschrieben werden
     * @param tlsClientId
     */
    virtual void workOnClosed_CaServer(const int tlsClientId) = 0;

    /**
     * Nachricht an einen TCP Client senden.
     * Der Client muss anhand seiner TCP ID identifiziert werden.
     * @param clientId
     * @param msg
     * @return bool (True := Senden erfolgreich, False := Senden fehlgeschlagen)
     */
    bool writeMsg(const int clientId, const std::string& msg) override final;

private:

    /**
     * Nichts weiter tun
     * @return int (0 := OK, ~0 := Fehler)
     */
    int init(const char* const,
             const char* const,
             const char* const) override final;

    /**
     * Nichts weiter tun
     */
    void deinit() override final;

    /**
     * Nichts weiter tun
     * @param clientId
     * @return int* (&clientId)
     */
    int* connectionInit(const int clientId) override final;

    /**
     * Nichts weiter tun
     * @param socket
     */
    void connectionDeinit(int* socket) override final;

    /**
     * Nächste empfangene Nachricht empfangen.
     * @param socket
     * @return string ("" := Empfangen fehlgeschlagen, sonst := Inhalt der Nachricht)
     */
    std::string readMsg(int* socket) override final;

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

    // Object kann nicht kopiert werden
    CaServer(const CaServer&) = delete;
    CaServer& operator = (const CaServer&) = delete;
};
}

#endif // CASERVER_H
