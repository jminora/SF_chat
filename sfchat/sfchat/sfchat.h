#pragma once
#include <string>
#include <unordered_map>
#include <memory>

using namespace std;

class UserAccount {
private:
    string login;
    string passwordHash;
    string name;

public:
    UserAccount() = default;
    UserAccount(string login, string passwordHash, string name);

    const string& getLogin() const;
    const string& getPasswordHash() const;
    const string& getName() const;

    void setLogin(const string& value);
    void setPasswordHash(const string& value);
    void setName(const string& value);
};

string getCurrentTime();

void clearScreen();

class App {
public:
    static void loadUsers(unordered_map<string, pair<string, string>>& users);
    static bool isUserLoginExist(const unordered_map<string, pair<string, string>>& users, const string& login);
    static bool isUserNameExist(const unordered_map<string, pair<string, string>>& users, const string& name);
    static bool authProcess(unordered_map<string, pair<string, string>>& users, unique_ptr<UserAccount>& user);
    static string hashPassword(const string& password);
};