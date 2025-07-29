#include <fstream>
#include <limits>
#include "tcpclient.h"
#include "chatwork.h"
#include "logger.h"

int main() {
	TCPClient chatclient;
	Logger logger;

	std::unordered_map<std::string, std::pair<std::string, std::string>> users;
	App::loadUsers(users);

	std::unique_ptr<UserAccount> user = std::make_unique<UserAccount>();
	
	while (!App::authProcess(users, user, chatclient)) {
		// repeat until authorized
	}
	logger.writeLog(*user, "logged in");
	
	clearScreen();
	
	runChatLoop(chatclient, users, user, logger);

	return 0;
}
