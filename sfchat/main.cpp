#include <fstream>
#include <limits>
#include "sfchat.h"
#include "tcpclient.h"
#include "chatwork.h"

int main() {
	TCPClient chatclient;

	std::unordered_map<std::string, std::pair<std::string, std::string>> users;
	App::loadUsers(users);

	std::unique_ptr<UserAccount> user = std::make_unique<UserAccount>();
	
	while (!App::authProcess(users, user, chatclient)) {
		// repeat until authorized
	}
	
	clearScreen();
	
	runChatLoop(chatclient, users, user);

	return 0;
}
