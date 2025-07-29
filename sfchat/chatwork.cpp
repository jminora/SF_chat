#include <iostream>
#include <fstream>
#include <limits>
#include <sstream>
#include <thread>
#include "chatwork.h"

void runChatLoop(
	TCPClient &chatclient,
	std::unordered_map<std::string, std::pair<std::string, std::string>> &users,
	std::unique_ptr<UserAccount> &user,
	Logger& logger
	)

{
	bool working = true;
	char choice;
	std::string input, pmName;

	while (working)
	{
		std::cout << "Hi, " << user->getName() << "! Choose an option:\n";
		std::cout << "(1) Read general chat\n(2) Write to general chat\n";
		std::cout << "(3) Read PM\n(4) Send PM\n(5) Read Logs\n(6) Log out\n(7) Exit\nChoice: ";
		std::cin >> choice;
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		clearScreen();

		switch (choice)
		{
		case '1':
		{
			chatclient.download();
			std::ifstream chat("chat.txt");
			if (chat)
			{
				std::cout << std::string(std::istreambuf_iterator<char>(chat), {});
			}
			else
				std::cerr << "Failed to open chat.txt\n";
			logger.writeLog(*user, "read general chat");
			std::cout << "Press Enter to continue...";
			std::cin.get();
			clearScreen();
			break;
		}
		case '2':
		{
			std::cout << "Enter your message: ";
			getline(std::cin, input);
			chatclient.download();
			{
				std::ofstream chat("chat.txt", std::ios::app);
				chat << "[" << getCurrentTime() << "] " << user->getName() << ": " << input << '\n';
			}
			logger.writeLog(*user, "wrote in general chat");
			clearScreen();
			chatclient.upload();
			break;
		}
		case '3':
		{
			std::cout << "Read PM from: ";
			getline(std::cin, pmName);
			if (!App::isUserNameExist(users, pmName))
			{
				std::cout << "User not found\n";
				std::cout << "Press Enter to continue...";
				std::cin.get();
				clearScreen();
				break;
			}
			std::string key1 = "From " + pmName + " to " + user->getName();
			std::string key2 = "From " + user->getName() + " to " + pmName;
			chatclient.download();
			std::ifstream pchat("pchat.txt");
			while (getline(pchat, input))
			{
				if (input.find(key1) != std::string::npos || input.find(key2) != std::string::npos)
				{
					std::cout << input << '\n';
				}
			}
			logger.writeLog(*user, "read PM from " + pmName);
			std::cout << "Press Enter to continue...";
			std::cin.get();
			clearScreen();
			break;
		}
		case '4':
		{
			std::cout << "Send PM to: ";
			getline(std::cin, pmName);
			if (!App::isUserNameExist(users, pmName))
			{
				std::cout << "User not found\n";
				std::cout << "Press Enter to continue...";
				std::cin.get();
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
			logger.writeLog(*user, "sent PM to " + pmName);
			clearScreen();
			chatclient.upload();
			break;
		}
		case '5':
		{
			std::thread t_write(&Logger::writeLog, &logger, std::ref(*user), "read logs");
			t_write.join();
			std::thread t_read(&Logger::readLog, &logger);
			t_read.join();
			std::cout << "Press Enter to continue...";
			std::cin.get();
			clearScreen();
			break;
		}
		case '6':
		{
			logger.writeLog(*user, "logged out");
			user = std::make_unique<UserAccount>();
			while (!App::authProcess(users, user, chatclient))
			{
			}
			logger.writeLog(*user, "logged in");
			clearScreen();
			break;
		}
		case '7':
			working = false;
			break;
		default:
			std::cout << "Invalid choice\n";
			std::cout << "Press Enter to continue...";
			std::cin.get();
			clearScreen();
		}
	}
}
