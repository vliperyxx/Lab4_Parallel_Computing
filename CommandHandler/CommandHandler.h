#pragma once
#include <winsock2.h>
#include <string>

class CommandHandler {
public:
    static bool sendCommand(SOCKET socket, const std::string& command);
    static bool receiveCommand(SOCKET socket, std::string& command);
    static bool sendInt(SOCKET socket, int value);
    static bool receiveInt(SOCKET socket, int& value);
};