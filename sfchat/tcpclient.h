#pragma once

#include <string>
#include <vector>

#ifdef _WIN32
#define NOMINMAX
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
typedef int socklen_t;
#else
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
typedef int SOCKET;
#endif

class TCPClient {
public:
    TCPClient();
    ~TCPClient();

    void upload();
    void download();

private:
    SOCKET clientSock;

    void initSockets();
    void cleanupSockets();
    void closeSocket(SOCKET sock);

    bool connectToServer();
    void uploadFile(const std::string& filename);
    void downloadFile(const std::string& filename);
    void sendQuit();
};
