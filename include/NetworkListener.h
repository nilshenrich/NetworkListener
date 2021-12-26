/**
 * Grundgerüst für alle Klassen, die in irgendeiner Form einen Netzwerkserver basierend auf TCP bilden.
 * Diese Klasse enthält selbst keine Funktionalität, sondern bildet lediglich ein Gerüst
 * für das einfache Erstellen stabiler Server aus TCP Basis.
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

namespace HS_Networking
{
class networking_error: public std::exception
{
public:
    networking_error(std::string msg = "unexpected networking error"): msg{msg} {}
    virtual ~networking_error() {}

    const char* what()
    {
        return msg.c_str();
    }

private:

    std::string msg;

    // Default Konstruktor löschen
    networking_error() = delete;

    // Nicht kopieren
    networking_error(const networking_error&) = delete;
    networking_error& operator = (const networking_error&) = delete;
};

template<class SocketType, class SocketDeleter = std::default_delete<SocketType>>
class NetworkListener
{
public:
    NetworkListener() {}
    virtual ~NetworkListener()
    {
        stop();
    }

    /**
     * Starten des Listeners.
     * Beim Ausführen der Methode "listenerStart()" wird ein Hintergrundprozess gestartet,
     * der dauerhaft neue eingehende Verbindungsanfragen von Cients annimmt.
     * @param port
     * @param pathToCaCert
     * @param pathToCert
     * @param pathToPrivKey
     * @return int (0 := OK, ~0 := Fehler)
     */
    virtual int start(const int port,
                      const char* const pathToCaCert = nullptr,
                      const char* const pathToCert = nullptr,
                      const char* const pathToPrivKey = nullptr);

    /**
     * Beenden des Listeners.
     * Beendet das Annehmen neuer Verbindungsanfragen und trennt alle laufenden Verbindungen.
     */
    void stop();

    /**
     * Nachricht an einen Client senden.
     * Der Client muss anhand seiner TCP ID identifiziert werden.
     * Diese Methode prepariert die Zeichenkette für das Versenden und ruft die spezielle Methode auf, um diese zu versenden
     * @param clientId
     * @param msg
     * @return bool (True := Senden erfolgreich, False := Senden fehlgeschlagen)
     */
    bool sendMsg(const int clientId, const std::string& msg);

    /**
     * IP Adresse des Clients ausgeben
     * @param clientId
     * @return string
     */
    std::string getClientIp(const int clientId);

protected:

    /**
     * Initialisieren des Listeners.
     * Diese Methode wird vor dem Start des Listeners aufgerufen und muss in erbenden Klassen implementiert werden.
     * Hier sollen alle Einstellungen des Listeners vorgenommen werden.
     * @param pathToCaCert
     * @param pathToCert
     * @param pathToPrivKey
     * @return int (0 := OK, ~0 := Fehler)
     */
    virtual int init(const char* const pathToCaCert,
                     const char* const pathToCert,
                     const char* const pathToPrivKey) = 0;

    /**
     * Deinitialisieren des Listeners.
     * Diese Methode wird nach dem Beenden des Listeners aufgerufen und muss in erbenden Klassen implementiert werden.
     * Hier sollen Schritte definiert werden, um den Listener volständig zu beenden.
     */
    virtual void deinit() = 0;

    /**
     * Verbindung initialisieren.
     * Wenn eine neue TCP Verbindung hergestellt wurde, wird diese Methode unmittelbar danach aufgerufen.
     * @param clientId
     * @return SocketType* (nullptr := Fehler -> Empfangen abbrechen, Sonst := Initialisierung erfolgreich -> Nachrichten empfangen)
     */
    virtual SocketType* connectionInit(const int clientId) = 0;

    /**
     * Verbindung deinitialisieren.
     * Wenn eine bestehende TCP Verbindung abreißt, wird diese Methode unmittelbar danach aufgerufen.
     * @param socket
     */
    virtual void connectionDeinit(SocketType* socket) = 0;

    /**
     * Nächste empfangene Nachricht empfangen.
     * Diese Methode muss von erbenden Klassen implementiert werden.
     * @param socket
     * @return string ("" := Empfangen fehlgeschlagen, sonst := Inhalt der Nachricht)
     */
    virtual std::string readMsg(SocketType* socket) = 0;

    /**
     * Nachricht an einen Client senden.
     * Der Client muss anhand seiner TCP ID identifiziert werden.
     * Diese Methode muss von erbenden Klassen implementiert werden.
     * @param clientId
     * @param msg
     * @return bool (True := Senden erfolgreich, False := Senden fehlgeschlagen)
     */
    virtual bool writeMsg(const int clientId, const std::string& msg) = 0;

    /**
     * Diese Methode wird aufgerufen, wenn eine neue Nachricht von einem verbundenen Client empfangen wurde.
     * Diese Methode muss in erbenden Klassen implementiert werden.
     * Diese Methode wird in einem eigenen Prozess gestartet.
     * Die Bearbeitungsdauer dieser Methode kann also beliebig lang sein.
     * @param clientId
     * @param msg
     */
    virtual void workOnMessage(const int clientId, const std::string msg) = 0;

    /**
     * Diese Methode wird aufgerufen, wenn eine bestehende Verbindung abgerissen ist oder getrennt wurde.
     * Diese Methode muss in erbenden Klassen implementiert werden.
     * Diese Methode wird in einem eigenen Prozess gestartet.
     * Die Bearbeitungsdauer dieser Methode kann also beliebig lang sein.
     * @param clientId
     */
    virtual void workOnClosed(const int clientId) = 0;

    // Map, die jede aktive Verbindung mit deren TCP ID identifiziert
    std::map<int, std::unique_ptr<SocketType, SocketDeleter>> activeConnections {};

    // Mutex für aktive Verbindungen
    std::mutex activeConnections_m {};

    // Maximale Paketgröße, die empfangen werden kann
    const static int MAXIMUM_RECEIVE_PACKAGE_SIZE {16384};

private:

    /**
     * Annehmen eingehender Verbindungsanfragen.
     * Diese Methode läuft in einer Dauerschleife bis "listenerStop()" aufgerufen wird.
     * Für jede angenommene TCP Verbindung wird eine dauerhafter Leseroutine in einem weiteren Prozess gestartet.
     */
    void listenerAccept();

    /**
     * Empfangen und Verarbeiten eingehender Nachrichten von einem bestimmten TCP Client.
     * Für jede aktive TCP Verbindung läuft eine eigene Laufzeit dieser Funktion dauerhaft.
     * @param clientId
     */
    void listenerReceive(const int clientId);

    // Flag, das wiedergibt, ob der Listener läuft
    bool running {false};

    // TCP Socket Adresse des Listeners
    struct sockaddr_in socketAddress {};

    // TCP Socket des Listeners
    int tcpSocket {0};

    // Prozess, in dem das Hören auf Verbindungsanfragen läuft
    std::thread accHandler {};

    // Object kann nicht kopiert werden
    NetworkListener(const NetworkListener&) = delete;
    NetworkListener& operator = (const NetworkListener&) = delete;
};

template<class SocketType, class SocketDeleter>
int NetworkListener<SocketType, SocketDeleter>::start(
    const int port,
    const char* const pathToCaCert,
    const char* const pathToCert,
    const char* const pathToPrivKey)
{
    using namespace std;

    // Prüfen, ob der Server bereits läuft
    if (running)
    {
#ifdef DEVELOP
        cerr << typeid(this).name() << "::" << __func__ << ": Listener läuft bereits" << endl;
#endif // DEVELOP

        return -1;
    }

    // Prüfen, ob der Port im vorgesehenen Bereich liegt
    if (1 > port || 65535 < port)
    {
#ifdef DEVELOP
        cerr << typeid(this).name() << "::" << __func__ << ": Der Port " << port << " kann nicht verwendet werden" << endl;
#endif // DEVELOP

        return NETWORKING_ERROR_START_WRONG_PORT;
    }

    // Listener initialisieren
    int initCode {init(pathToCaCert, pathToCert, pathToPrivKey)};
    if (initCode)
        return initCode;

    // Server TCP Socket erstellen
    tcpSocket = socket(AF_INET, SOCK_STREAM, 0);

    // TCP Socket überprüfen
    if (-1 == tcpSocket)
    {
#ifdef DEVELOP
        cerr << typeid(this).name() << "::" << __func__ << ": Server TCP Socket des Listeners konnte nicht erstellt werden" << endl;
#endif // DEVELOP

        // Server beenden
        stop();

        return NETWORKING_ERROR_START_CREATE_SOCKET;
    }

    // Socketoptionen setzen
    int opt {0};
    if (setsockopt(tcpSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
    {
#ifdef DEVELOP
        cerr << typeid(this).name() << "::" << __func__ << ": Die Optionen für den TCP Socket des Listeners konnten nicht gesetzt werden" << endl;
#endif // DEVELOP

        // Server beenden
        stop();

        return NETWORKING_ERROR_START_SET_SOCKET_OPT;
    }

    // Server TCP Socket Adresse initialisieren
    memset(&socketAddress, 0, sizeof(socketAddress));
    socketAddress.sin_family = AF_INET;
    socketAddress.sin_addr.s_addr = INADDR_ANY;
    socketAddress.sin_port = htons(port);

    // Socket an den Port binden
    if (bind(tcpSocket, (struct sockaddr*)&socketAddress, sizeof(socketAddress)))
    {
#ifdef DEVELOP
        cerr << typeid(this).name() << "::" << __func__ << ": Server TCP Socket des Listeners konnte nicht an den Port " << port << " gebunden werden" << endl;
#endif // DEVELOP

        // Server beenden
        stop();

        return NETWORKING_ERROR_START_BIND_PORT;
    }

    // Listener starten
    if (listen(tcpSocket, SOMAXCONN))
    {
#ifdef DEVELOP
        cerr << typeid(this).name() << "::" << __func__ << ": Listener konnte nicht gestartet werden" << endl;
#endif // DEVELOP

        // Server stoppen
        stop();

        return NETWORKING_ERROR_START_LISTENER;
    }

    // Listener läuft jetzt
    running = true;

    // Auf neue Verbindungsanfragen hören im Hintergrund
    if (accHandler.joinable())
        throw networking_error("Starten des Listeners: Es läuft bereits ein Hintergrundprozess, um Verbindungsanfragen anzunehmen");
    accHandler = thread{&NetworkListener::listenerAccept, this};

    return initCode;
}

template<class SocketType, class SocketDeleter>
void NetworkListener<SocketType, SocketDeleter>::stop()
{
    using namespace std;

    // Listener beenden
    running = false;

    // Listener TCP Socket blockieten. Damit werden alle read Funktionen abgebrochen
    if (shutdown(tcpSocket, SHUT_RDWR))
        return;

    // Warten bis der accept-Prozess beendet ist
    if (accHandler.joinable())
        accHandler.join();

    // Listener TCP Socket beenden
    close(tcpSocket);

    // Listener deinitialisieren
    deinit();

    return;
}

template<class SocketType, class SocketDeleter>
bool NetworkListener<SocketType, SocketDeleter>::sendMsg(const int clientId, const std::string& msg)
{
    using namespace std;

    // Beginnende und abschließende Zeichen um Nachricht setzen und versenden
    return writeMsg(clientId, string {NETWORKING_CHAR_TRANSFER_START} + msg + string {NETWORKING_CHAR_TRANSFER_END});
}

template<class SocketType, class SocketDeleter>
std::string NetworkListener<SocketType, SocketDeleter>::getClientIp(const int clientId)
{
    using namespace std;

    struct sockaddr_in addr;
    socklen_t addrSize = sizeof(struct sockaddr_in);
    if (getpeername(clientId, (struct sockaddr*)&addr, &addrSize))
    {
#ifdef DEVELOP
        cerr << typeid(this).name() << "::" << __func__ << ": IP Adresse des Clients " << clientId << " konnte nicht gelesen werden" << endl;
#endif // DEVELOP

        return "Failed Read!";

    }

    // Gelesene IP Adresse in String umwandeln und zurückgeben
    return string{inet_ntoa(addr.sin_addr)};
}

template<class SocketType, class SocketDeleter>
void NetworkListener<SocketType, SocketDeleter>::listenerAccept()
{
    using namespace std;

    // Länge der Socket Adresse des Lsiteners (Wichtig für Verbindungsaufbau)
    socklen_t socketAddress_len {sizeof(socketAddress)};

    // Auf Verbindungsanfragen hören bis der Listener beendet wird
    while (running)
    {
        // Auf neue Verbindungsanfrage warten
        const int newConnection {accept(tcpSocket, (struct sockaddr*)&socketAddress, &socketAddress_len)};

        // Wenn die ID der neuen Verbindung -1 ist, ist die Verbindung fehlgeschlagen.
        // In diesem Fall soll die Schleife von neuem begonnen werden.
        if (-1 == newConnection)
            continue;

#ifdef DEVELOP
        cout << typeid(this).name() << "::" << __func__ << ": Neuer Client verbunden: " << newConnection << endl;
#endif // DEVELOP

        // Bei erfolgreicher Verbindung, sollen dauerhaft eingehende Nachrichten dieser Verbindung in einem neuen Prozess gelesen werden
        thread {&NetworkListener::listenerReceive, this, newConnection}.detach();
    }

    // Alle aktiven Client Sockets beenden
    {
        lock_guard<mutex> lck {activeConnections_m};
        for (const auto& it: activeConnections)
        {
            shutdown(it.first, SHUT_RDWR);

#ifdef DEVELOP
            cout << typeid(this).name() << "::" << __func__ << ": Verbindung zu TLS Client " << it.first << " beendet" << endl;
#endif // DEVELOP
        }
    }

    // Auf das Beenden aller receive Prozesse warten
    // Bis map der Verbindungen leer ist
    while (!activeConnections.empty())
        this_thread::sleep_for(100ms);

    return;
}

template<class SocketType, class SocketDeleter>
void NetworkListener<SocketType, SocketDeleter>::listenerReceive(const int clientId)
{
    using namespace std;

    // Neue Verbindung initialisieren
    SocketType* connection_p {connectionInit(clientId)};
    if (!connection_p)
        return;

    // Verbindung zu aktiven Verbindungen hinzufügen
    {
        lock_guard<mutex> lck {activeConnections_m};
        activeConnections[clientId] = unique_ptr<SocketType, SocketDeleter> {connection_p};
    }

    // Eingehende Nachrichten dieser Verbindung lesen, solange der Listener läuft
    string buffer;
    while (1)
    {
        // Auf neue eingehende Nachricht warten
        string msg {readMsg(connection_p)};
        if (msg.empty())
        {
#ifdef DEVELOP
            cout << typeid(this).name() << "::" << __func__ << ": Verbindung zu Client " << clientId << " abgerissen" << endl;
#endif // DEVELOP

            {
                lock_guard<mutex> lck {activeConnections_m};

                // Verbindung deinitialisieren
                connectionDeinit(connection_p);

                // Socket herunterfahren
                shutdown(clientId, SHUT_RDWR);

                // Verbindung aus Map entfernen
                activeConnections.erase(clientId);
            }

            // Wenn die empfangene Nachricht leer ist, ist die Verbindung abgerissen
            workOnClosed(clientId);

            // TCP Socket schließen
            close(clientId);

            // Prozess beenden, indem die Methode beendet wird
            return;
        }

        // Die eigentliche Nachricht ist zwischen STX und ETX im durchgängigen Stream eingeschlossen
        // Nachricht herausfiltern
        for (char c: msg)
        {
            switch (c)
            {
            // Start der Nachricht. -> Puffer leeren
            case NETWORKING_CHAR_TRANSFER_START:
                buffer.clear();
                break;

            // Ende der Nachricht. -> Bearbeiten (Durch move wird Puffer direkt geleert)
            case NETWORKING_CHAR_TRANSFER_END:
#ifdef DEVELOP
                cout << typeid(this).name() << "::" << __func__ << ": Nachricht von Client " << clientId << " empfangen: " << msg << endl;
#endif // DEVELOP
                thread {&NetworkListener::workOnMessage, this, clientId, move(buffer)}.detach();
                break;

            // Mitte der Nachricht. -> Zeichen puffern
            default:
                buffer.push_back(c);
                break;
            }
        }
    }
}
}

#endif // NETWORKLISTENER_H
