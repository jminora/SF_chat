#include "sfchat.h"
#include <iostream>
#include <fstream>
#include <chrono>
#include <ctime>

UserAccount::UserAccount(string login, string passwordHash, string name)
    : login(move(login)), passwordHash(move(passwordHash)), name(move(name)) {
}

const string& UserAccount::getLogin() const { return login; }
const string& UserAccount::getPasswordHash() const { return passwordHash; }
const string& UserAccount::getName() const { return name; }

void UserAccount::setLogin(const string& value) { login = value; }
void UserAccount::setPasswordHash(const string& value) { passwordHash = value; }
void UserAccount::setName(const string& value) { name = value; }

string getCurrentTime() {
    time_t now = chrono::system_clock::to_time_t(chrono::system_clock::now());
    tm localTime;
#ifdef _WIN32
    localtime_s(&localTime, &now);
#else
    localtime_r(&now, &localTime);
#endif
    char buffer[20];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &localTime);
    return string(buffer);
}

void clearScreen() {
    cout << "\033[2J\033[1;1H";
}

void App::loadUsers(unordered_map<string, pair<string, string>>& users) {
    ifstream file("users.txt");
    if (!file.is_open()) {
        cerr << "Failed to open users.txt\n";
        return;
    }
    string login, passwordHash, name;
    while (getline(file, login)) {
        if (!getline(file, passwordHash)) break;
        if (!getline(file, name)) break;
        users[login] = make_pair(passwordHash, name);
    }
}

bool App::isUserLoginExist(const unordered_map<string, pair<string, string>>& users, const string& login) {
    return users.find(login) != users.end();
}

bool App::isUserNameExist(const unordered_map<string, pair<string, string>>& users, const string& name)
{
    for (const auto& entry : users) {
        if (entry.second.second == name) return true;
    }
    return false;
}

bool App::authProcess(unordered_map<string, pair<string, string>>& users, unique_ptr<UserAccount>& user) {
    char choice;
    cout << "Welcome to SFChat!\n(1) Create account\n(2) Login\n(3) Exit\nChoice: ";
    cin >> choice;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    clearScreen();

    switch (choice) {
    case '1': {
        string login, password, name;
        do {
            cout << "Enter login: ";
            getline(cin, login);
            if (isUserLoginExist(users, login)) {
                cout << "Login already taken.\n";
            }
        } while (isUserLoginExist(users, login));

        cout << "Enter password: ";
        getline(cin, password);
        do {
            cout << "Enter name: ";
            getline(cin, name);
            if (isUserNameExist(users, name)) {
                cout << "Name already taken.\n";
            }
        } while (isUserNameExist(users, name));

        string hashed = hashPassword(password);

        ofstream file("users.txt", ios::app);
        if (!file.is_open()) {
            cerr << "Failed to open users.txt for writing\n";
            return false;
        }
        file << login << "\n" << hashed << "\n" << name << "\n";
        file.close();

        users[login] = { hashed, name };

        user = make_unique<UserAccount>(login, hashed, name);
        return true;
    }
    case '2': {
        string login, password;
        do {
            cout << "Enter login: ";
            getline(cin, login);
            if (!isUserLoginExist(users, login)) {
                cout << "User does not exist.\n";
                cout << "Press Enter to continue..."; cin.get();
                clearScreen();
            }
        } while (!isUserLoginExist(users, login));

        cout << "Enter password: ";
        getline(cin, password);
        string hashed = hashPassword(password);

        if (users.at(login).first == hashed) {
            cout << "Authorization successful.\n";
            user = make_unique<UserAccount>(login, hashed, users.at(login).second);
            return true;
        }
        else {
            cout << "Incorrect password.\n";
            cout << "Press Enter to continue..."; cin.get();
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

string App::hashPassword(const string& password) {
    constexpr uint64_t fnv_offset_basis = 14695981039346656037ULL;
    constexpr uint64_t fnv_prime = 1099511628211ULL;
    uint64_t hash = fnv_offset_basis;

    for (unsigned char c : password) {
        hash ^= c;
        hash *= fnv_prime;
    }

    const char* hex_digits = "0123456789abcdef";
    string result(16, '0');

    for (int i = 15; i >= 0; --i) {
        result[i] = hex_digits[hash & 0xF];
        hash >>= 4;
    }

    return result;
}

