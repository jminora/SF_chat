#pragma once

#include <string>

#ifdef _WIN32
#include <winsock2.h>
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

class TCPServer {
public:
	TCPServer(int port);
	~TCPServer();

	bool start();                        // Start the server and listen for connections
	void handleClient(SOCKET clientSock); // Handle a single client session

private:
	int port;
	SOCKET serverSock;

	void initSockets();    // Platform-specific socket initialization
	void cleanupSockets(); // Platform-specific socket cleanup
	void closeSocket(SOCKET sock); // Close a socket
};
