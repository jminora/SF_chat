#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include "tcpclient.h"

class UserAccount {
private:
    std::string login;
    std::string passwordHash;
    std::string name;

public:
    UserAccount() = default;
    UserAccount(std::string login, std::string passwordHash, std::string name);

    const std::string& getLogin() const;
    const std::string& getPasswordHash() const;
    const std::string& getName() const;

    void setLogin(const std::string& value);
    void setPasswordHash(const std::string& value);
    void setName(const std::string& value);
};

std::string getCurrentTime();

void clearScreen();

class App {
public:
    static void loadUsers(std::unordered_map<std::string, std::pair<std::string, std::string>>& users);
    static bool isUserLoginExist(const std::unordered_map<std::string, std::pair<std::string, std::string>>& users, const std::string& login);
    static bool isUserNameExist(const std::unordered_map<std::string, std::pair<std::string, std::string>>& users, const std::string& name);
    static bool authProcess(std::unordered_map<std::string, std::pair<std::string, std::string>>& users, std::unique_ptr<UserAccount>& user, TCPClient& chatclient);
    static std::string hashPassword(const std::string& password);
};