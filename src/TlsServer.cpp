#include "TlsServer.h"

using namespace HS_Networking;
using namespace std;

TlsServer::TlsServer() {}

TlsServer::~TlsServer() {}

int TlsServer::init(const char* const pathToCaCert,
                    const char* const pathToCert,
                    const char* const pathToPrivKey)
{
    // OpenSSL initialisieren
    OpenSSL_add_ssl_algorithms();

    // Verschlüsselungsmethode festlegen (Neueste verfügbare Version von TLS serverseitig)
    if (!(serverContext = SSL_CTX_new(TLS_server_method())))
    {
#ifdef DEVELOP
        cerr << typeid(this).name() << "::" << __func__ << ": Verschlüsselungsmethode kann nicht festgelegt werden" << endl;
#endif // DEVELOP

        // Server beenden
        stop();

        return NETWORKING_ERROR_START_SET_CONTEXT;
    }

    // CA Zertifikat für Server laden
    if (1 != SSL_CTX_load_verify_locations(serverContext, pathToCaCert, nullptr))
    {
#ifdef DEVELOP
        cerr << typeid(this).name() << "::" << __func__ << ": CA Zertifikat \"" << pathToCaCert << "\" kann nicht geladen werden" << endl;
#endif // DEVELOP

        // Server beenden
        stop();

        return NETWORKING_ERROR_START_WRONG_CA_PATH;
    }

    // CA Zertifikat für Client laden
    SSL_CTX_set_client_CA_list(serverContext, SSL_load_client_CA_file(pathToCaCert));

    // Server Zertifikat laden
    if (1 != SSL_CTX_use_certificate_file(serverContext, pathToCert, SSL_FILETYPE_PEM))
    {
#ifdef DEVELOP
        cerr << typeid(this).name() << "::" << __func__ << ": Server Zertifikat \"" << pathToCert << "\" konnte nicht geladen werden" << endl;
#endif // DEVELOP

        // Server beenden
        stop();

        return NETWORKING_ERROR_START_WRONG_CERT_PATH;
    }

    // Server Private Key laden
    if (1 != SSL_CTX_use_PrivateKey_file(serverContext, pathToPrivKey, SSL_FILETYPE_PEM))
    {
#ifdef DEVELOP
        cerr << typeid(this).name() << "::" << __func__ << ": Server Private Key \"" << pathToPrivKey << "\" konnte nicht geladen werden" << endl;
#endif // DEVELOP

        // Server beenden
        stop();

        return NETWORKING_ERROR_START_WRONG_KEY_PATH;
    }

    // Prüfen, ob das Zertifikat und der Private Key zusammen passen
    if (1 != SSL_CTX_check_private_key(serverContext))
    {
#ifdef DEVELOP
        cerr << typeid(this).name() << "::" << __func__ << ": Server Private Key passt nicht zum Zertifikat" << endl;
#endif // DEVELOP

        // Server beenden
        stop();

        return NETWORKING_ERROR_START_WRONG_KEY;
    }

    // TLS Modus festlegen
    SSL_CTX_set_mode(serverContext, SSL_MODE_AUTO_RETRY);

    // Authentifizierung des Clients fordern
    SSL_CTX_set_verify(serverContext, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT | SSL_VERIFY_CLIENT_ONCE, NULL);

    // Signatur des Client Zertifikats prüfen (Muss zum CA Zertifikat passen)
    SSL_CTX_set_verify_depth(serverContext, 1);

    return NETWORKING_START_OK;
}

void TlsServer::deinit()
{
    SSL_CTX_free(serverContext);
    serverContext = nullptr;

    return;
}

SSL* TlsServer::connectionInit(const int clientId)
{
    // TLS Kanal erstellen
    SSL* tlsSocket{SSL_new(serverContext)};

    // Prüfen, ob das Erstellen des TLS Kanals erfolgreich war
    if (!tlsSocket)
    {
#ifdef DEVELOP
        cerr << typeid(this).name() << "::" << __func__ << ": TLS Kanal konnte nicht erstellt werden" << endl;
#endif // DEVELOP

        // TCP Verbindung schließen
        shutdown(clientId, SHUT_RDWR);
        close(clientId);
        SSL_free(tlsSocket);

        return nullptr;
    }

    // TLS Kanal an neue TCP Verbindung anbinden
    if (!SSL_set_fd(tlsSocket, clientId))
    {
#ifdef DEVELOP
        cerr << typeid(this).name() << "::" << __func__ << ": TLS Kanal konnte nicht an TCP Verbindung gebunden werden" << endl;
#endif // DEVELOP

        // TCP Verbindung schließen
        SSL_shutdown(tlsSocket);
        shutdown(clientId, SHUT_RDWR);
        close(clientId);
        SSL_free(tlsSocket);

        return nullptr;
    }

    // TLS Handshake durchführen
    if (1 != SSL_accept(tlsSocket))
    {
#ifdef DEVELOP
        cerr << typeid(this).name() << "::" << __func__ << ": TLS Handshake fehlgeschlagen" << endl;
#endif // DEVELOP

        // TCP Verbindung schließen
        SSL_shutdown(tlsSocket);
        shutdown(clientId, SHUT_RDWR);
        close(clientId);
        SSL_free(tlsSocket);

        return nullptr;
    }

#ifdef DEVELOP
    cout << typeid(this).name() << "::" << __func__ << ": Neuer TLS Client erfolgreich verbunden: " << clientId << endl;
#endif // DEVELOP

    return tlsSocket;
}

bool TlsServer::writeMsg(const int clientId, const std::string& msg)
{
    // Nachricht in char array umwandeln (utf8)
    const char* const buffer = msg.c_str();

    // Länge der zu sendenden Nachricht
    const int lenMsg {(int)msg.size()};

    lock_guard<mutex> lck {activeConnections_m};

    // Prüfen, ob die Verbindung als aktiv gespeichert ist
    if (activeConnections.end() == activeConnections.find(clientId))
    {
#ifdef DEVELOP
        cerr << typeid(this).name() << "::" << __func__ << ": Der Client " << clientId << " ist nicht  verbunden" << endl;
#endif // DEVELOP

        return false;
    }

#ifdef DEVELOP
    cout << typeid(this).name() << "::" << __func__ << ": An Client " << clientId << " senden: " << msg << endl;
#endif // DEVELOP

    // TLS Kanal des Clients, an den die Nachricht gehen soll, auslesen
    SSL* socket{activeConnections[clientId].get()};

    // Nachricht senden
    // War das Senden erfolgreicht, entspricht der Rückgabewert der Länge der Nachricht
    return SSL_write(socket, buffer, lenMsg) == lenMsg;
}

string TlsServer::readMsg(SSL* socket)
{
    // Puffer für eingehende Nachrichten
    char buffer[MAXIMUM_RECEIVE_PACKAGE_SIZE] {0};

    // Auf TLS Nachricht warten
    const int lenMsg {SSL_read(socket, buffer, MAXIMUM_RECEIVE_PACKAGE_SIZE)};

    // Nachricht zurückgeben (Bei Länge 0 oder kleiner "")
    return string{buffer, 0 < lenMsg ?  static_cast<size_t>(lenMsg) : 0UL};
}

void TlsServer::connectionDeinit(SSL* socket)
{
    SSL_shutdown(socket);
    return;
}

void TlsServer::workOnMessage(const int clientId, const string msg)
{
    // Spezielle Bearbeitungsmethode ausführen
    workOnMessage_TlsServer(clientId, move(msg));
    return;
}

void TlsServer::workOnClosed(const int clientId)
{
    // Spezielle Bearbeitungsmethode ausführen
    workOnClosed_TlsServer(clientId);
    return;
}

string TlsServer::getSubjPartFromClientCert(const int clientId, const SSL* tlsSocket, const int subjPart)
{
    char buf[256] {0};

    // Wenn der TLS Socket NULL ist, Socket aus Liste der verbundenen Clients auslesen
    if(!tlsSocket)
    {
        lock_guard<mutex> lck {activeConnections_m};
        if (activeConnections.find(clientId) == activeConnections.end())
        {
#ifdef DEVELOP
            cerr << typeid(this).name() << "::" << __func__ << ": Kein Client mit der ID " << clientId << " verbunden" << endl;
#endif // DEVELOP

            return "";
        }

        tlsSocket = activeConnections[clientId].get();
    }

    // Client Zertifikat aus bestehender TLS Verbindung auslesen
    unique_ptr<X509, void(*)(X509*)> remoteCert{SSL_get_peer_certificate(tlsSocket), X509_free};

    // Kompletten Subject String aus Zertifikat auslesen
    X509_NAME* remoteCertSubject{X509_get_subject_name(remoteCert.get())};

    // Common Name aus Subject String auslesen und in Puffer schreiben
    X509_NAME_get_text_by_NID(remoteCertSubject, subjPart, buf, 256);

    // Common Name in String schreiben
    return string(buf);
}

