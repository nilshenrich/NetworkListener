#include "TcpServer.h"

using namespace networking;
using namespace std;

TcpServer::TcpServer(char delimiter) : NetworkListener{delimiter} {}

TcpServer::~TcpServer()
{
    stop();
}

int TcpServer::init(const char *const,
                    const char *const,
                    const char *const)
{
    return NETWORKLISTENER_START_OK;
}

int *TcpServer::connectionInit(const int clientId)
{
    return new int{clientId};
}

bool TcpServer::writeMsg(const int clientId, const std::string &msg)
{
#ifdef DEVELOP
    cout << typeid(this).name() << "::" << __func__ << ": Send to client " << clientId << ": " << msg << endl;
#endif // DEVELOP

    const size_t lenMsg{msg.size()};
    return send(clientId, msg.c_str(), lenMsg, 0) == (ssize_t)lenMsg;
}

string TcpServer::readMsg(int *socket)
{
    // Buffer for received data.
    char buffer[MAXIMUM_RECEIVE_PACKAGE_SIZE]{0};

    // Wait for data to be available.
    ssize_t lenMsg{recv(*socket, buffer, MAXIMUM_RECEIVE_PACKAGE_SIZE, 0)};

    // Return received data as string (or empty string if no data is available).
    return string{buffer, 0 < lenMsg ? static_cast<size_t>(lenMsg) : 0UL};
}

void TcpServer::connectionDeinit(int *)
{
    return;
}

void TcpServer::workOnMessage(const int clientId, const string msg)
{
    workOnMessage_TcpServer(clientId, move(msg));
    return;
}

void TcpServer::workOnClosed(const int clientId)
{
    workOnClosed_TcpServer(clientId);
    return;
}
