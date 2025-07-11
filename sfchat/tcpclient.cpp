#include "tcpclient.h"
#include <fstream>
#include <iostream>
#include <sstream>

std::string server_ip = "127.0.0.1";
int server_port = 9000;
const int BUFFER_SIZE = 4096;

TCPClient::TCPClient() : clientSock(INVALID_SOCKET) {
	initSockets();
}

TCPClient::~TCPClient() {
	if (clientSock != INVALID_SOCKET)
		closeSocket(clientSock);
	cleanupSockets();
}

void TCPClient::initSockets() {
#ifdef _WIN32
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
}

void TCPClient::cleanupSockets() {
#ifdef _WIN32
	WSACleanup();
#endif
}

void TCPClient::closeSocket(SOCKET sock) {
#ifdef _WIN32
	closesocket(sock);
#else
	close(sock);
#endif
}

bool TCPClient::connectToServer() {
	readConfig();
	clientSock = socket(AF_INET, SOCK_STREAM, 0);
	if (clientSock == INVALID_SOCKET) {
		std::cerr << "Socket creation failed.\n";
		return false;
	}

	sockaddr_in server_addr{};
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(server_port);

#ifdef _WIN32
	InetPtonA(AF_INET, server_ip.c_str(), &server_addr.sin_addr);
#else
	inet_pton(AF_INET, server_ip.c_str(), &server_addr.sin_addr);
#endif

	if (connect(clientSock, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
		std::cerr << "Connection failed.\n";
		return false;
	}

	//std::cout << "Connected to server " << server_ip << ":" << server_port << "\n";
	return true;
}

void TCPClient::uploadFile(const std::string& filename) {
	std::ifstream in(filename, std::ios::binary);
	if (!in) {
		std::cerr << "Cannot open file for upload: " << filename << "\n";
		return;
	}

	in.seekg(0, std::ios::end);
	std::streamsize fileSize = in.tellg();
	in.seekg(0, std::ios::beg);

	std::ostringstream cmd;
	cmd << "UPLOAD " << filename << " " << fileSize << "\n";
	std::string cmdStr = cmd.str();
	send(clientSock, cmdStr.c_str(), cmdStr.length(), 0);

	char buffer[BUFFER_SIZE];
	while (in.read(buffer, BUFFER_SIZE) || in.gcount() > 0) {
		send(clientSock, buffer, in.gcount(), 0);
	}

	//std::cout << "Uploaded: " << filename << "\n";

	// Wait for server confirmation
	std::string response;
	char ch;
	while (recv(clientSock, &ch, 1, 0) > 0) {
		if (ch == '\n') break;
		response += ch;
	}

	if (response == "OK") {
		//std::cout << "Server confirmed receipt of: " << filename << "\n";
	}
	else {
		std::cerr << "Server reported failure for: " << filename << " (" << response << ")\n";
	}
}


void TCPClient::downloadFile(const std::string& filename) {
	std::ostringstream cmd;
	cmd << "DOWNLOAD " << filename << "\n";
	send(clientSock, cmd.str().c_str(), cmd.str().length(), 0);

	std::string filesize_str;
	char ch;
	int result;
	while ((result = recv(clientSock, &ch, 1, 0)) > 0) {
		if (ch == '\n') break;
		filesize_str += ch;
	}
	if (result <= 0) {
		std::cerr << "Connection closed while reading file size\n";
		return;
	}

	size_t filesize = std::stoull(filesize_str);

	std::ofstream out(filename, std::ios::binary);
	if (!out) {
		std::cerr << "Cannot create file for download: " << filename << "\n";
		return;
	}

	size_t totalReceived = 0;
	char buffer[BUFFER_SIZE];
	while (totalReceived < filesize) {
		int toRead = (filesize - totalReceived) < BUFFER_SIZE ? (filesize - totalReceived) : BUFFER_SIZE;
		int bytesReceived = recv(clientSock, buffer, toRead, 0);
		if (bytesReceived <= 0) {
			std::cerr << "Connection lost during file download.\n";
			break;
		}
		out.write(buffer, bytesReceived);
		totalReceived += bytesReceived;
	}

	if (totalReceived == filesize) {
		//std::cout << "Downloaded: " << filename << "\n";
	}
	else {
		std::cerr << "Download incomplete: " << filename << "\n";
	}
}

void TCPClient::sendQuit() {
	const std::string quitCmd = "QUIT\n";
	send(clientSock, quitCmd.c_str(), quitCmd.length(), 0);
}

void TCPClient::readConfig()
{
	std::string input{};
	std::ifstream config("config.txt");

	if (!config.is_open())
	{
		std::cerr << "Failed to open config.txt" << std::endl;
		return;
	}

	while (getline(config, input)) {
		std::istringstream iss(input);
		std::string key{}, value{};

		if (std::getline(iss, key, '=') && std::getline(iss, value))
		{
			if (key == "server_ip") 
			{
				server_ip = value;
			}
			else if (key == "server_port") 
			{
				server_port = std::stoi(value);
			}
		}
	}

	config.close();
}

void TCPClient::upload() {
	if (!connectToServer())
		return;

	// Files to upload
	std::vector<std::string> uploadList = {
		"chat.txt",
		"pchat.txt",
		"users.txt"
	};

	for (const auto& file : uploadList) {
		uploadFile(file);
	}

	sendQuit();
}

void TCPClient::download() {
	if (!connectToServer())
		return;

	// Files to download
	std::vector<std::string> downloadList = {
		"chat.txt",
		"pchat.txt",
		"users.txt"
	};

	for (const auto& file : downloadList) {
		downloadFile(file);
	}

	sendQuit();
}
