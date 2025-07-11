#include "tcpserver.h"
#include "db_transfer.h"
#include <iostream>
#include <fstream>
#include <sstream>

const int BUFFER_SIZE = 4096;

TCPServer::TCPServer(int port) : port(port), serverSock(INVALID_SOCKET) {
    initSockets();
}

TCPServer::~TCPServer() {
    if (serverSock != INVALID_SOCKET)
        closeSocket(serverSock);
    cleanupSockets();
}

void TCPServer::initSockets() {
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
}

void TCPServer::cleanupSockets() {
#ifdef _WIN32
    WSACleanup();
#endif
}

void TCPServer::closeSocket(SOCKET sock) {
#ifdef _WIN32
    closesocket(sock);
#else
    close(sock);
#endif
}

bool TCPServer::start() {
    serverSock = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSock == INVALID_SOCKET) {
        std::cerr << "Socket creation failed.\n";
        return false;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSock, (sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "Bind failed.\n";
        return false;
    }

    listen(serverSock, 1);
    std::cout << "Server started on port " << port << "...\n";

    while (true) {
        sockaddr_in clientAddr{};
        socklen_t len = sizeof(clientAddr);
        SOCKET clientSock = accept(serverSock, (sockaddr*)&clientAddr, &len);
        if (clientSock == INVALID_SOCKET) {
            std::cerr << "Accept failed.\n";
            continue;
        }

        std::cout << "Client connected.\n";
        handleClient(clientSock);
        closeSocket(clientSock);
        std::cout << "Client disconnected.\n";
    }

    return true;
}

void TCPServer::handleClient(SOCKET clientSock) {
    char buffer[BUFFER_SIZE];
    DbTransfer db;

    while (true) {
        std::string command_line;
        char ch;
        int result;

        while ((result = recv(clientSock, &ch, 1, 0)) > 0) {
            if (ch == '\n') break;
            command_line += ch;
        }

        if (result <= 0) {
            std::cout << "Connection closed by client.\n";
            break;
        }

        std::istringstream iss(command_line);
        std::string command, filename;
        size_t filesize = 0;
        iss >> command >> filename >> filesize;

        if (command == "UPLOAD") {
            std::ofstream out(filename, std::ios::binary);
            if (!out) {
                std::cerr << "Failed to create file: " << filename << "\n";
                continue;
            }

            size_t totalReceived = 0;
            while (totalReceived < filesize) {
                int toRead = (filesize - totalReceived) < BUFFER_SIZE ? (filesize - totalReceived) : BUFFER_SIZE;
                int bytesReceived = recv(clientSock, buffer, toRead, 0);
                if (bytesReceived <= 0) {
                    std::cerr << "Connection lost during file reception.\n";
                    break;
                }
                out.write(buffer, bytesReceived);
                totalReceived += bytesReceived;
            }

            if (totalReceived == filesize) {
                std::cout << "File received successfully: " << filename << "\n";
                send(clientSock, "OK\n", 3, 0);
            }
            else {
                std::cerr << "File reception incomplete: " << filename << "\n";
                send(clientSock, "FAIL\n", 5, 0);
            }
        }
        else if (command == "DOWNLOAD") {
            std::ifstream in(filename, std::ios::binary);
            if (!in) {
                std::cerr << "File not found: " << filename << "\n";
                continue;
            }

            in.seekg(0, std::ios::end);
            size_t filesize = in.tellg();
            in.seekg(0, std::ios::beg);

            std::ostringstream header;
            header << filesize << "\n";
            std::string headerStr = header.str();
            send(clientSock, headerStr.c_str(), headerStr.size(), 0);

            char buffer[BUFFER_SIZE];
            while (in.read(buffer, BUFFER_SIZE) || in.gcount() > 0) {
                send(clientSock, buffer, in.gcount(), 0);
            }

            std::cout << "File sent: " << filename << "\n";
        }
        else if (command == "QUIT") {
            std::cout << "Client requested to close connection.\n";
            db.doDBTransfer();
            break;
        }
        else {
            std::cerr << "Unknown command: " << command << "\n";
        }
    }
}
