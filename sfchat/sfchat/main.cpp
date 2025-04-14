#include <iostream>
#include <fstream>
#include "sfchat.h"

int main()
{
	bool working{ true };
	char answer{};
	string user_post{};
	string current_fstring{};
	string pm_name{};
	string keyword1{};
	string keyword2{};
	fstream chat;
	fstream pchat;
	unique_ptr<User_Account> userAccount(new User_Account());

	while (!authProcess(userAccount)) //User authorization process
	{
		authProcess(userAccount);
	};
		
	cout << "\033[2J\033[1;1H"; //Clearing the screen

	while (working)
	{
		cout << "Hi, " << userAccount->getUser_name() << "! What you want to do?: " << endl << "(1) Read a general chat" << endl << "(2) Write to a general chat" << endl << "(3) Read PM from a user" << endl << "(4) PM to a specific user" << endl << "(5) Log out" << endl << "(6) Exit" << endl << "Your answer?:  ";
		cin >> answer;

		cout << "\033[2J\033[1;1H"; //Clearing the screen

		switch (answer)
		{
		case '1':
		{
			//read from a chat
			chat.open("chat.txt");
			
			if (!chat.is_open()) //check file availability
			{
				cerr << "Error while opening file." << endl;
				exit(EXIT_FAILURE);
			}

			string chat_contents{ istreambuf_iterator<char>(chat), istreambuf_iterator<char>() }; //Read all lines from chat.txt
			cout << chat_contents << endl;
			chat.close();
			cin.ignore();
			cout << "Press Enter to continue.." << endl;
			cin.get();
			cout << "\033[2J\033[1;1H"; //Clearing the screen
			break;
		}

		case '2':
		{
			//write to a chat
			chat.open("chat.txt", ios::app); //open with writing from the end
			
			if (!chat.is_open()) //check file availability
			{
				cerr << "Error while opening file." << endl;
				exit(EXIT_FAILURE);
			}

			getline(cin >> ws, user_post);
			//Write down the time, username and message to chat.txt
			chat << "[" << getCurrentTime() << "] " << userAccount->getUser_name() << ": " << user_post << endl;
			chat.close();
			cout << "\033[2J\033[1;1H"; //Clearing the screen
			break;
		}

		case '3':
		{
			do //Check for user existence
			{
				cout << "Enter name of the user you want to read a PM from: ";
				getline(cin >> ws, pm_name);
				if (!isUserExist(pm_name)) 
				{
					cout << "This user does not exist" << endl;
				};
			} while (!isUserExist(pm_name));

			keyword1 = "From " + pm_name + " to " + userAccount->getUser_name(); //keywords to search messages from and for the user
			keyword2 = "From " + userAccount->getUser_name() + " to " + pm_name;
			
			pchat.open("pchat.txt");
			
			if (!pchat.is_open()) //check file availability
			{
				cerr << "Error while opening file." << endl;
				exit(EXIT_FAILURE);
			}
			
			while (getline(pchat, current_fstring)) { //filtering and displaying messages by keywords
				if ((current_fstring.find(keyword1) != string::npos) || (current_fstring.find(keyword2) != string::npos)) {
					cout << current_fstring << endl;
				}
			}
			
			pchat.close();
			cout << endl << "Press Enter to continue.." << endl;
			cin.get();
			cout << "\033[2J\033[1;1H"; //Clearing the screen
			break;
		}

		case '4':
		{
			do //Check for user existence
			{
				cout << "Enter name of the user you want to send a PM to: ";
				getline(cin >> ws, pm_name);
				if (!isUserExist(pm_name))
				{
					cout << "This user does not exist" << endl;
				};
			} while (!isUserExist(pm_name));

			pchat.open("pchat.txt", ios::app);
			
			if (!pchat.is_open()) //check file availability
			{
				cerr << "Error while opening file." << endl;
				exit(EXIT_FAILURE);
			}

			getline(cin >> ws, user_post);
			pchat << "[" << getCurrentTime() << "] " << "From " << userAccount->getUser_name() << " to " << pm_name << ": " << user_post << endl;
			pchat.close();
			cout << "\033[2J\033[1;1H"; //Clearing the screen
			break;
		}

		case '5':
			while (!authProcess(userAccount))
			{
				authProcess(userAccount);
			};
			cout << "\033[2J\033[1;1H"; //Clearing the screen
			break;


		case '6':
			working = false;
			break;

		default:
			cout << "Error. Try again" << endl;
			break;
		}
	}
	return 0;
}
