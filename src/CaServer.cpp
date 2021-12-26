#include "CaServer.h"

using namespace HS_Networking;
using namespace std;

CaServer::CaServer() {}

CaServer::~CaServer() {}

int CaServer::init(const char* const,
                   const char* const,
                   const char* const)
{
    return NETWORKING_START_OK;
}

void CaServer::deinit()
{
    return;
}

int* CaServer::connectionInit(const int clientId)
{
    return new int {clientId};
}

bool CaServer::writeMsg(const int clientId, const std::string& msg)
{
#ifdef DEVELOP
    cout << typeid(this).name() << "::" << __func__ << ": An Client " << clientId << " senden: " << msg << endl;
#endif // DEVELOP

    const size_t lenMsg {msg.size()};
    return send(clientId, msg.c_str(), lenMsg, 0) == (ssize_t)lenMsg;
}

string CaServer::readMsg(int* socket)
{
    // Puffer für eingehende Nachrichten
    char buffer[MAXIMUM_RECEIVE_PACKAGE_SIZE] {0};

    // Auf TLS Nachricht warten
    ssize_t lenMsg {recv(*socket, buffer, MAXIMUM_RECEIVE_PACKAGE_SIZE, 0)};

    // Nachricht zurückgeben (Bei Länge 0 oder kleiner "")
    return string{buffer, 0 < lenMsg ?  static_cast<size_t>(lenMsg) : 0UL};
}

void CaServer::connectionDeinit(int*)
{
    return;
}

void CaServer::workOnMessage(const int clientId, const string msg)
{
    // Spezielle Bearbeitungsfunktion ausführen
    workOnMessage_CaServer(clientId, move(msg));
    return;
}

void CaServer::workOnClosed(const int clientId)
{
    // Spezielle Bearbeitungsfunktion ausführen
    workOnClosed_CaServer(clientId);
    return;
}
