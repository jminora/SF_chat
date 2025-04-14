#pragma once
#include <string>

using namespace std;

class User_Account
{
private:
	string user_login{};
	string user_password{};
	string user_name{};

public:
	User_Account();
	User_Account(string user_login, string user_password, string user_name);
	~User_Account();

	string getUser_login() const;
	void setUser_login(string value);

	string getUser_password() const;
	void setUser_password(string value);

	string getUser_name() const;
	void setUser_name(string value);
};

string getCurrentTime();

bool isUserExist(string);

bool authProcess(unique_ptr<User_Account>& ua_name);