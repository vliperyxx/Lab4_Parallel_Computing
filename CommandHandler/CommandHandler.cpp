#include "CommandHandler.h"

#pragma comment(lib, "ws2_32.lib")

bool CommandHandler::sendCommand(SOCKET socket, const std::string& command) {
    int length = htonl(command.size());
    if (send(socket, (char*)&length, sizeof(length), 0) == SOCKET_ERROR) {
        return false;
    }
    return send(socket, command.c_str(), command.size(), 0) != SOCKET_ERROR;
}

bool CommandHandler::receiveCommand(SOCKET socket, std::string& command) {
    int length = 0;
    if (recv(socket, (char*)&length, sizeof(length), 0) <= 0) {
        return false;
    }

    length = ntohl(length);
    char buffer[101];
    int received = recv(socket, buffer, length, 0);
    if (received <= 0) {
        return false;
    }
    command = std::string(buffer, received);
    return true;
}

bool CommandHandler::sendInt(SOCKET socket, int value) {
    int networkValue = htonl(value);
    return send(socket, (char*)&networkValue, sizeof(networkValue), 0) != SOCKET_ERROR;
}

bool CommandHandler::receiveInt(SOCKET socket, int& value) {
    int networkValue = 0;
    int bytes = recv(socket, (char*)&networkValue, sizeof(networkValue), 0);
    if (bytes <= 0) {
      return false;
    }

    value = ntohl(networkValue);
    return true;
}
