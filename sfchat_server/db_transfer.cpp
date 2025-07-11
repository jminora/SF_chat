#include "db_transfer.h"

#include <iostream>
#include <fstream>
#include <regex>
#include <stdexcept>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#include <sql.h>
#include <sqlext.h>
#else
#include <sql.h>
#include <sqlext.h>
#endif

struct DbTransfer::Impl {
	SQLHANDLE env = nullptr;
	SQLHANDLE dbc = nullptr;

	Impl() = default;

	void checkError(SQLRETURN retcode, SQLHANDLE handle, SQLSMALLINT type, const std::string& msg) {
		if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO) {
			SQLCHAR sqlState[1024] = { 0 };
			SQLCHAR message[1024] = { 0 };
			SQLSMALLINT length = 0;

			if (SQLGetDiagRecA(type, handle, 1, sqlState, nullptr, message, sizeof(message), &length) == SQL_SUCCESS) {
				throw std::runtime_error(msg + " - SQLSTATE: " + reinterpret_cast<char*>(sqlState) + ", Message: " + reinterpret_cast<char*>(message));
			}
			else {
				throw std::runtime_error(msg + " - Unknown ODBC error");
			}
		}
	}

	void connect(const std::string& dsn, const std::string& user, const std::string& pass) {
		SQLRETURN ret;

		ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env);
		checkError(ret, env, SQL_HANDLE_ENV, "Allocating environment handle");

		ret = SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0);
		checkError(ret, env, SQL_HANDLE_ENV, "Setting ODBC version");

		ret = SQLAllocHandle(SQL_HANDLE_DBC, env, &dbc);
		checkError(ret, dbc, SQL_HANDLE_DBC, "Allocating connection handle");

		ret = SQLConnectA(dbc,
			(SQLCHAR*)dsn.c_str(), SQL_NTS,
			(SQLCHAR*)user.c_str(), SQL_NTS,
			(SQLCHAR*)pass.c_str(), SQL_NTS);
		checkError(ret, dbc, SQL_HANDLE_DBC, "Connecting to database");
	}

	void disconnect() {
		if (dbc) {
			SQLDisconnect(dbc);
			SQLFreeHandle(SQL_HANDLE_DBC, dbc);
			dbc = nullptr;
		}
		if (env) {
			SQLFreeHandle(SQL_HANDLE_ENV, env);
			env = nullptr;
		}
	}

	void executePrepared(const char* sql,
		const std::vector<std::pair<SQLSMALLINT, std::string>>& params) {
		SQLHANDLE stmt;
		SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);
		checkError(ret, stmt, SQL_HANDLE_STMT, "Allocating statement handle");

		ret = SQLPrepareA(stmt, (SQLCHAR*)sql, SQL_NTS);
		checkError(ret, stmt, SQL_HANDLE_STMT, "Preparing statement");

		for (size_t i = 0; i < params.size(); ++i) {
			SQLSMALLINT paramNum = static_cast<SQLSMALLINT>(i + 1);
			SQLSMALLINT cType = SQL_C_CHAR;
			SQLSMALLINT sqlType = params[i].first;
			const std::string& val = params[i].second;

			ret = SQLBindParameter(stmt,
				paramNum,
				SQL_PARAM_INPUT,
				cType,
				sqlType,
				0,
				0,
				(SQLPOINTER)val.c_str(),
				0,
				NULL);
			checkError(ret, stmt, SQL_HANDLE_STMT, "Binding parameter " + std::to_string(paramNum));
		}

		ret = SQLExecute(stmt);
		checkError(ret, stmt, SQL_HANDLE_STMT, "Executing statement");

		SQLFreeHandle(SQL_HANDLE_STMT, stmt);
	}

	void executeSimpleSQL(const std::string& sql) {
		SQLHANDLE stmt;
		SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);
		checkError(ret, stmt, SQL_HANDLE_STMT, "Allocating statement handle");

		ret = SQLExecDirectA(stmt, (SQLCHAR*)sql.c_str(), SQL_NTS);
		checkError(ret, stmt, SQL_HANDLE_STMT, "Executing SQL: " + sql);

		SQLFreeHandle(SQL_HANDLE_STMT, stmt);
	}

	bool tableExists(const std::string& tableName) {
		std::string sql = "SELECT 1 FROM `" + tableName + "` LIMIT 1";

		SQLHANDLE stmt;
		SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);
		if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
			return false;
		}

		ret = SQLExecDirectA(stmt, (SQLCHAR*)sql.c_str(), SQL_NTS);
		if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
			SQLFreeHandle(SQL_HANDLE_STMT, stmt);
			return true;
		}
		SQLFreeHandle(SQL_HANDLE_STMT, stmt);
		return false;
	}

	void createUsersTable() {
		const char* sql =
			"CREATE TABLE users ("
			"login VARCHAR(64) PRIMARY KEY,"
			"password_hash VARCHAR(64) NOT NULL,"
			"name VARCHAR(100) NOT NULL UNIQUE"
			")";
		executeSimpleSQL(sql);
	}

	void createChatTable() {
		const char* sql =
			"CREATE TABLE chat ("
			"id INT AUTO_INCREMENT PRIMARY KEY,"
			"timestamp DATETIME NOT NULL,"
			"user_name VARCHAR(100) NOT NULL,"
			"message TEXT NOT NULL,"
			"FOREIGN KEY (user_name) REFERENCES users(name),"
			"UNIQUE KEY unique_chat (timestamp, user_name, message(100))"
			")";
		executeSimpleSQL(sql);
	}

	void createPChatTable() {
		const char* sql =
			"CREATE TABLE pchat ("
			"id INT AUTO_INCREMENT PRIMARY KEY,"
			"timestamp DATETIME NOT NULL,"
			"sender_name VARCHAR(100) NOT NULL,"
			"receiver_name VARCHAR(100) NOT NULL,"
			"message TEXT NOT NULL,"
			"FOREIGN KEY (sender_name) REFERENCES users(name),"
			"FOREIGN KEY (receiver_name) REFERENCES users(name),"
			"UNIQUE KEY unique_pchat (timestamp, sender_name, receiver_name, message(100))"
			")";
		executeSimpleSQL(sql);
	}
};

DbTransfer::DbTransfer() : pImpl(new Impl()) {}

DbTransfer::~DbTransfer() {
	if (pImpl) {
		pImpl->disconnect();
		delete pImpl;
		pImpl = nullptr;
	}
}

void DbTransfer::connect(const std::string& dsn, const std::string& user, const std::string& pass) {
	pImpl->connect(dsn, user, pass);
}

void DbTransfer::disconnect() {
	pImpl->disconnect();
}

void DbTransfer::checkAndCreateTables() {
	if (!pImpl->tableExists("users")) {
		std::cout << "Table 'users' not found. Creating...\n";
		pImpl->createUsersTable();
	}
	if (!pImpl->tableExists("chat")) {
		std::cout << "Table 'chat' not found. Creating...\n";
		pImpl->createChatTable();
	}
	if (!pImpl->tableExists("pchat")) {
		std::cout << "Table 'pchat' not found. Creating...\n";
		pImpl->createPChatTable();
	}
}

void DbTransfer::loadUsersFromFile(const std::string& filename) {
	std::ifstream file(filename);
	if (!file.is_open()) throw std::runtime_error("Cannot open file " + filename);

	std::string login, hash, name;
	while (std::getline(file, login) && std::getline(file, hash) && std::getline(file, name)) {
		const char* sql = "INSERT INTO users (login, password_hash, name) VALUES (?, ?, ?) ON DUPLICATE KEY UPDATE password_hash = VALUES(password_hash), name = VALUES(name)";

		std::vector<std::pair<SQLSMALLINT, std::string>> params = {
			{SQL_VARCHAR, login},
			{SQL_VARCHAR, hash},
			{SQL_VARCHAR, name}
		};
		pImpl->executePrepared(sql, params);
	}
}

void DbTransfer::loadChatFromFile(const std::string& filename) {
	std::ifstream file(filename);
	if (!file.is_open()) throw std::runtime_error("Cannot open file " + filename);

	std::regex regex(R"(\[(.*?)\]\s+(\w+):\s+(.*))");
	std::string line;
	while (std::getline(file, line)) {
		std::smatch match;
		if (std::regex_match(line, match, regex)) {
			std::string timestamp = match[1];
			std::string name = match[2];
			std::string message = match[3];

			const char* sql = "INSERT IGNORE INTO chat (timestamp, user_name, message) VALUES (?, ?, ?)";

			std::vector<std::pair<SQLSMALLINT, std::string>> params = {
				{SQL_TYPE_TIMESTAMP, timestamp},
				{SQL_VARCHAR, name},
				{SQL_LONGVARCHAR, message}
			};
			pImpl->executePrepared(sql, params);
		}
	}
}

void DbTransfer::loadPChatFromFile(const std::string& filename) {
	std::ifstream file(filename);
	if (!file.is_open()) throw std::runtime_error("Cannot open file " + filename);

	std::regex regex(R"(\[(.*?)\]\s+From\s+(\w+)\s+to\s+(\w+):\s+(.*))");
	std::string line;
	while (std::getline(file, line)) {
		std::smatch match;
		if (std::regex_match(line, match, regex)) {
			std::string timestamp = match[1];
			std::string sender = match[2];
			std::string receiver = match[3];
			std::string message = match[4];

			const char* sql = "INSERT IGNORE INTO pchat (timestamp, sender_name, receiver_name, message) VALUES (?, ?, ?, ?)";

			std::vector<std::pair<SQLSMALLINT, std::string>> params = {
				{SQL_TYPE_TIMESTAMP, timestamp},
				{SQL_VARCHAR, sender},
				{SQL_VARCHAR, receiver},
				{SQL_LONGVARCHAR, message}
			};
			pImpl->executePrepared(sql, params);
		}
	}
}


void DbTransfer::readConfig(const std::string& filename, std::string& dsn, std::string& user, std::string& pass) {
	std::ifstream file(filename);
	if (!file.is_open()) throw std::runtime_error("Cannot open config file: " + filename);

	std::string line;
	while (std::getline(file, line)) {
		if (line.empty() || line[0] == '#') continue;

		auto pos = line.find('=');
		if (pos == std::string::npos) continue;

		std::string key = line.substr(0, pos);
		std::string value = line.substr(pos + 1);

		// Remove extra spaces
		auto trim = [](std::string& s) {
			s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) { return !std::isspace(ch); }));
			s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), s.end());
			};

		trim(key);
		trim(value);

		if (key == "dsn") dsn = value;
		else if (key == "user") user = value;
		else if (key == "pass") pass = value;
	}

	if (dsn.empty()) throw std::runtime_error("DSN not specified in config file");
	if (user.empty()) throw std::runtime_error("User not specified in config file");
}

void DbTransfer::doDBTransfer()
{
	std::string dsn, user, pass;

	DbTransfer::readConfig("config.txt", dsn, user, pass);
	DbTransfer::connect(dsn, user, pass);

	DbTransfer::checkAndCreateTables();

	// Upload users.txt to DB
	DbTransfer::loadUsersFromFile("users.txt");
	std::cout << "Users loaded successfully.\n";

	// Upload chat.txt to DB
	DbTransfer::loadChatFromFile("chat.txt");
	std::cout << "Chat messages loaded successfully.\n";

	// Upload pchat.txt to DB
	DbTransfer::loadPChatFromFile("pchat.txt");
	std::cout << "Private chat messages loaded successfully.\n";

	DbTransfer::disconnect();
	std::cout << "Database connection closed.\n";
}
