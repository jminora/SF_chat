#include "logger.h"

Logger::Logger()
{
    logfile.open("log.txt", std::ios::in | std::ios::out | std::ios::app);
    if (!logfile.is_open())
    {
        std::cerr << "Failed to open log.txt\n";
        return;
    }
}

Logger::~Logger()
{
    logfile.close();
}

void Logger::writeLog(const UserAccount& user, const std::string &message)
{
    if (!logfile.is_open())
    {
        std::cerr << "Failed to open log.txt\n";
        return;
    }
    shared_mutex.lock();
    logfile.clear();
    logfile.seekp(0, std::ios::end);
    logfile << "[" << getCurrentTime() << "] " << user.getName() << " " << message << '\n';
    logfile.flush();
    shared_mutex.unlock();
}

void Logger::readLog()
{
    if (!logfile.is_open())
    {
        std::cerr << "Failed to open log.txt\n";
        return;
    }
    shared_mutex.lock_shared();
    logfile.clear();
    logfile.seekg(0, std::ios::beg);
    std::cout << std::string(std::istreambuf_iterator<char>(logfile), {});
    shared_mutex.unlock_shared();
}
