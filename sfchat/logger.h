#pragma once

#include <fstream>
#include <string>
#include <mutex>
#include <shared_mutex>
#include <iostream>
#include "sfchat.h"

class Logger
{
public:
    Logger();
    ~Logger();

    void writeLog(const UserAccount& user, const std::string& message);
    void readLog();

private:
    std::fstream logfile;
    std::shared_mutex shared_mutex;
};