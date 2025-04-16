#pragma once
#include <winsock2.h>
#include <string>
#include <vector>

class CommandHandler {
public:
    static bool sendCommand(SOCKET socket, const std::string& command);
    static bool receiveCommand(SOCKET socket, std::string& command);
    static bool sendInt(SOCKET socket, int value);
    static bool receiveInt(SOCKET socket, int& value);
    static bool sendMatrixChunked(SOCKET socket, const std::vector<int>& matrix, int chunkSize);
    static bool receiveMatrixChunked(SOCKET socket, std::vector<int>& matrix);
    static bool sendStatus(SOCKET socket, const std::string& status, int progress = -1);
    static bool receiveStatus(SOCKET socket, std::string& status, int& progress);
};