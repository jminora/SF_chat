#include <fstream>
#include <limits>
#include <iostream>
#include "sfchat.h"

using namespace std;

int main() {
    unordered_map<string, pair<string, string>> users;
    App::loadUsers(users);

    unique_ptr<UserAccount> user = make_unique<UserAccount>();

    while (!App::authProcess(users, user)) {
        // repeat until authorized
    }

    clearScreen();

    bool working = true;
    char choice;
    string input, pmName;

    while (working) {
        cout << "Hi, " << user->getName() << "! Choose an option:\n";
        cout << "(1) Read general chat\n(2) Write to general chat\n";
        cout << "(3) Read PM\n(4) Send PM\n(5) Log out\n(6) Exit\nChoice: ";
        cin >> choice;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        clearScreen();

        switch (choice) {
        case '1': {
            ifstream chat("chat.txt");
            if (chat) {
                cout << string(istreambuf_iterator<char>(chat), {});
            }
            else cerr << "Failed to open chat.txt\n";
            cout << "Press Enter to continue..."; cin.get();
            clearScreen();
            break;
        }
        case '2': {
            cout << "Enter your message: ";
            getline(cin, input);
            ofstream chat("chat.txt", ios::app);
            chat << "[" << getCurrentTime() << "] " << user->getName() << ": " << input << '\n';
            clearScreen();
            break;
        }
        case '3': {
            cout << "Read PM from: ";
            getline(cin, pmName);
            if (!App::isUserNameExist(users, pmName)) {
                cout << "User not found\n";
                cout << "Press Enter to continue..."; cin.get();
                clearScreen();
                break;
            }
            string key1 = "From " + pmName + " to " + user->getName();
            string key2 = "From " + user->getName() + " to " + pmName;
            ifstream pchat("pchat.txt");
            while (getline(pchat, input)) {
                if (input.find(key1) != string::npos || input.find(key2) != string::npos) {
                    cout << input << '\n';
                }
            }
            cout << "Press Enter to continue..."; cin.get();
            clearScreen();
            break;
        }
        case '4': {
            cout << "Send PM to: ";
            getline(cin, pmName);
            if (!App::isUserNameExist(users, pmName)) {
                cout << "User not found\n";
                cout << "Press Enter to continue..."; cin.get();
                clearScreen();
                break;
            }
            cout << "Enter your message: ";
            getline(cin, input);
            ofstream pchat("pchat.txt", ios::app);
            pchat << "[" << getCurrentTime() << "] From " << user->getName() << " to " << pmName << ": " << input << '\n';
            clearScreen();
            break;
        }
        case '5': {
            user = make_unique<UserAccount>();
            while (!App::authProcess(users, user)) {}
            clearScreen();
            break;
        }
        case '6':
            working = false;
            break;
        default:
            cout << "Invalid choice\n";
            cout << "Press Enter to continue..."; cin.get();
            clearScreen();
        }
    }
    return 0;
}
