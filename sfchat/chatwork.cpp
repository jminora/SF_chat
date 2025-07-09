#include "chatwork.h"
#include <iostream>
#include <fstream>
#include <limits>
#include <sstream>

void runChatLoop(
	TCPClient& chatclient,
	std::unordered_map<std::string, std::pair<std::string, std::string>>& users,
	std::unique_ptr<UserAccount>& user
) {
	bool working = true;
	char choice;
	std::string input, pmName;

	while (working) {
		std::cout << "Hi, " << user->getName() << "! Choose an option:\n";
		std::cout << "(1) Read general chat\n(2) Write to general chat\n";
		std::cout << "(3) Read PM\n(4) Send PM\n(5) Log out\n(6) Exit\nChoice: ";
		std::cin >> choice;
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		clearScreen();

		while (working) {
			std::cout << "Hi, " << user->getName() << "! Choose an option:\n";
			std::cout << "(1) Read general chat\n(2) Write to general chat\n";
			std::cout << "(3) Read PM\n(4) Send PM\n(5) Log out\n(6) Exit\nChoice: ";
			std::cin >> choice;
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			clearScreen();

			switch (choice) {
			case '1': {
				chatclient.download();
				std::ifstream chat("chat.txt");
				if (chat) {
					std::cout << std::string(std::istreambuf_iterator<char>(chat), {});
				}
				else std::cerr << "Failed to open chat.txt\n";
				std::cout << "Press Enter to continue..."; std::cin.get();
				clearScreen();
				break;
			}
			case '2': {
				std::cout << "Enter your message: ";
				getline(std::cin, input);
				chatclient.download();
				{
					std::ofstream chat("chat.txt", std::ios::app);
					chat << "[" << getCurrentTime() << "] " << user->getName() << ": " << input << '\n';
				}
				clearScreen();
				chatclient.upload();
				break;
			}
			case '3': {
				std::cout << "Read PM from: ";
				getline(std::cin, pmName);
				if (!App::isUserNameExist(users, pmName)) {
					std::cout << "User not found\n";
					std::cout << "Press Enter to continue..."; std::cin.get();
					clearScreen();
					break;
				}
				std::string key1 = "From " + pmName + " to " + user->getName();
				std::string key2 = "From " + user->getName() + " to " + pmName;
				chatclient.download();
				std::ifstream pchat("pchat.txt");
				while (getline(pchat, input)) {
					if (input.find(key1) != std::string::npos || input.find(key2) != std::string::npos) {
						std::cout << input << '\n';
					}
				}
				std::cout << "Press Enter to continue..."; std::cin.get();
				clearScreen();
				break;
			}
			case '4': {
				std::cout << "Send PM to: ";
				getline(std::cin, pmName);
				if (!App::isUserNameExist(users, pmName)) {
					std::cout << "User not found\n";
					std::cout << "Press Enter to continue..."; std::cin.get();
					clearScreen();
					break;
				}
				std::cout << "Enter your message: ";
				getline(std::cin, input);
				chatclient.download();
				{
					std::ofstream pchat("pchat.txt", std::ios::app);
					pchat << "[" << getCurrentTime() << "] From " << user->getName() << " to " << pmName << ": " << input << '\n';
				}
				clearScreen();
				chatclient.upload();
				break;
			}
			case '5': {
				user = std::make_unique<UserAccount>();
				while (!App::authProcess(users, user)) {}
				clearScreen();
				break;
			}
			case '6':
				working = false;
				break;
			default:
				std::cout << "Invalid choice\n";
				std::cout << "Press Enter to continue..."; std::cin.get();
				clearScreen();
			}
		}
	}
}
