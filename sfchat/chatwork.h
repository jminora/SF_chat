#pragma once

#include "tcpclient.h"
#include "sfchat.h"

void runChatLoop(
    TCPClient& chatclient,
    std::unordered_map<std::string, std::pair<std::string, std::string>>& users,
    std::unique_ptr<UserAccount>& user
);