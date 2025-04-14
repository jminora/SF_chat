#include <chrono>
#include <ctime>
#include <iostream>
#include <fstream>
#include "sfchat.h"

using namespace std;

User_Account::User_Account()
{
}

User_Account::User_Account(string user_login, string user_password, string user_name)
{
	this->user_login = user_login;
	this->user_password = user_password;
	this->user_name = user_name;
}

User_Account::~User_Account()
{
}

string User_Account::getUser_login() const
{
	return user_login;
}

void User_Account::setUser_login(string value)
{
	user_login = value;
}

string User_Account::getUser_password() const
{
	return user_password;
}

void User_Account::setUser_password(string value)
{
	user_password = value;
}

string User_Account::getUser_name() const
{
	return user_name;
}

void User_Account::setUser_name(string value)
{
	user_name = value;
}

string getCurrentTime()
{
	string current_time{};
	time_t time = chrono::system_clock::to_time_t(chrono::system_clock::now());
	tm local_time;
	localtime_s(&local_time, &time);
	char time_buffer[100];
	strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", &local_time);
	current_time = time_buffer;

	return current_time;
}

bool isUserExist(string value)
{
	fstream users;
	string current_fstring{};

	users.open("users.txt"); 
	if (!users.is_open()) //check file availability
	{
		cerr << "Error while opening file." << endl;
		exit(EXIT_FAILURE);
	}

	while (getline(users, current_fstring))
	{
		if (current_fstring == value)
		{
			users.close();
			return true;
		}
	}
	users.close();
	return false;
}

bool authProcess(unique_ptr<User_Account>& ua_name)
{
	string login{};
	string password{};
	string name{};
	string current_fstring{};
	char answer{};
	fstream users;

	cout << "Welcome to SFChat!" << endl << "(1) Create a account" << endl << "(2) Login to your account" << endl << "(3) Exit" << endl << "Your answer?: ";
	cin >> answer;

	cout << "\033[2J\033[1;1H";

	switch (answer)
	{
	case '1':
	{
		do //Check for user existence
		{
			cout << "Enter your login: ";
			getline(cin >> ws, login);
			if (isUserExist(login))
			{
				cout << "This name is already taken" << endl;
			};
		} while (isUserExist(login));

		cout << "Enter your password: ";
		getline(cin >> ws, password);
		cout << "Enter your name: ";
		getline(cin >> ws, name);

		ua_name->setUser_login(login);
		ua_name->setUser_password(password);
		ua_name->setUser_name(name);

		users.open("users.txt", ios::app); //open with writing from the end
		if (!users.is_open()) //check file availability
		{
			cerr << "Error while opening file." << endl;
			exit(EXIT_FAILURE);
		}
		users << login << endl << password << endl << name << endl;
		users.close();
		return 1;

		break;
	}
	case '2':
	{
		do //Check for user existence
		{
			cout << "Enter your login: ";
			getline(cin >> ws, login);
			if (!isUserExist(login))
			{
				cout << "This user does not exist" << endl;
			};
		} while (!isUserExist(login));

		users.open("users.txt");
		if (!users.is_open()) //check file availability
		{
			cerr << "Error while opening file." << endl;
			exit(EXIT_FAILURE);
		}

		while (users >> current_fstring)
		{
			if (current_fstring == login)
			{
				cout << "Enter your password: ";
				getline(cin >> ws, password);
				users.ignore();
				getline(users, current_fstring);
				if (current_fstring == password)
				{
					cout << "Successful authorization" << endl;

					ua_name->setUser_login(login);
					ua_name->setUser_password(password);
					getline(users, current_fstring);
					ua_name->setUser_name(current_fstring);

					break;
				}
				else
				{
					cout << "Incorrect password" << endl;
					users.close();
					cout << endl << "Press Enter to continue.." << endl;
					cin.get();
					cout << "\033[2J\033[1;1H"; //Clearing the screen

					return 0;
				}

			}
		}
		users.close();
		return 1;

		break;
	}
	case '3':
		exit(0);

	default:
		return 0;
		break;
	}
}
