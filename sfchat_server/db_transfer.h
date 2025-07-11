#pragma once
#include <string>

class DbTransfer {
public:
	DbTransfer();
	~DbTransfer();

	void connect(const std::string& dsn, const std::string& user, const std::string& pass);
	void disconnect();

	void checkAndCreateTables();

	void loadUsersFromFile(const std::string& filename);
	void loadChatFromFile(const std::string& filename);
	void loadPChatFromFile(const std::string& filename);

	void readConfig(const std::string& filename, std::string& dsn, std::string& user, std::string& pass);

	void doDBTransfer();
private:
	struct Impl;
	Impl* pImpl;
};
