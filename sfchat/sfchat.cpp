#include "sfchat.h"
#include "chatwork.h"
#include <iostream>
#include <fstream>
#include <chrono>
#include <ctime>

UserAccount::UserAccount(std::string login, std::string passwordHash, std::string name)
    : login(move(login)), passwordHash(move(passwordHash)), name(move(name)) {
}

const std::string& UserAccount::getLogin() const { return login; }
const std::string& UserAccount::getPasswordHash() const { return passwordHash; }
const std::string& UserAccount::getName() const { return name; }

void UserAccount::setLogin(const std::string& value) { login = value; }
void UserAccount::setPasswordHash(const std::string& value) { passwordHash = value; }
void UserAccount::setName(const std::string& value) { name = value; }

std::string getCurrentTime() {
    time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    tm localTime;
#ifdef _WIN32
    localtime_s(&localTime, &now);
#else
    localtime_r(&now, &localTime);
#endif
    char buffer[20];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &localTime);
    return std::string(buffer);
}

void clearScreen() {
    std::cout << "\033[2J\033[1;1H";
}

void App::loadUsers(std::unordered_map<std::string, std::pair<std::string, std::string>>& users) {
    std::ifstream file("users.txt");
    if (!file.is_open()) {
        std::cerr << "Failed to open users.txt\n";
        return;
    }
    std::string login, passwordHash, name;
    while (getline(file, login)) {
        if (!getline(file, passwordHash)) break;
        if (!getline(file, name)) break;
        users[login] = make_pair(passwordHash, name);
    }
}

bool App::isUserLoginExist(const std::unordered_map<std::string, std::pair<std::string, std::string>>& users, const std::string& login) {
    return users.find(login) != users.end();
}

bool App::isUserNameExist(const std::unordered_map<std::string, std::pair<std::string, std::string>>& users, const std::string& name)
{
    for (const auto& entry : users) {
        if (entry.second.second == name) return true;
    }
    return false;
}

bool App::authProcess(std::unordered_map<std::string, std::pair<std::string, std::string>>& users, std::unique_ptr<UserAccount>& user, TCPClient& chatclient) {
    char choice;
    std::cout << "Welcome to SFChat!\n(1) Create account\n(2) Login\n(3) Exit\nChoice: ";
    std::cin >> choice;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    chatclient.download();
    clearScreen();
    
    switch (choice) {
    case '1': {
        std::string login, password, name;
        do {
            std::cout << "Enter login: ";
            getline(std::cin, login);
            if (isUserLoginExist(users, login)) {
                std::cout << "Login already taken.\n";
            }
        } while (isUserLoginExist(users, login));

        std::cout << "Enter password: ";
        getline(std::cin, password);
        do {
            std::cout << "Enter name: ";
            getline(std::cin, name);
            if (isUserNameExist(users, name)) {
                std::cout << "Name already taken.\n";
            }
        } while (isUserNameExist(users, name));

        std::string hashed = hashPassword(password);

        std::ofstream file("users.txt", std::ios::app);
        if (!file.is_open()) {
            std::cerr << "Failed to open users.txt for writing\n";
            return false;
        }
        file << login << "\n" << hashed << "\n" << name << "\n";
        file.close();

        users[login] = { hashed, name };

        user = std::make_unique<UserAccount>(login, hashed, name);
        chatclient.upload();
        
        return true;
    }
    case '2': {
        std::string login, password;
        do {
            std::cout << "Enter login: ";
            getline(std::cin, login);
            if (!isUserLoginExist(users, login)) {
                std::cout << "User does not exist.\n";
                std::cout << "Press Enter to continue..."; std::cin.get();
                clearScreen();
            }
        } while (!isUserLoginExist(users, login));

        std::cout << "Enter password: ";
        getline(std::cin, password);
        std::string hashed = hashPassword(password);

        if (users.at(login).first == hashed) {
            std::cout << "Authorization successful.\n";
            user = std::make_unique<UserAccount>(login, hashed, users.at(login).second);
            return true;
        }
        else {
            std::cout << "Incorrect password.\n";
            std::cout << "Press Enter to continue..."; std::cin.get();
            clearScreen();
            return false;
        }
    }
    case '3':
        exit(0);
    default:
        return false;
    }
}

std::string App::hashPassword(const std::string& password) {
    constexpr uint64_t fnv_offset_basis = 14695981039346656037ULL;
    constexpr uint64_t fnv_prime = 1099511628211ULL;
    uint64_t hash = fnv_offset_basis;

    for (unsigned char c : password) {
        hash ^= c;
        hash *= fnv_prime;
    }

    const char* hex_digits = "0123456789abcdef";
    std::string result(16, '0');

    for (int i = 15; i >= 0; --i) {
        result[i] = hex_digits[hash & 0xF];
        hash >>= 4;
    }

    return result;
}

