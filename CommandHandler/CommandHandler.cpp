#include "CommandHandler.h"
#include <algorithm>

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

bool CommandHandler::sendMatrixChunked(SOCKET socket, const std::vector<int>& matrix, int chunkSize) {
    int totalSize = matrix.size();
    if (!sendInt(socket, totalSize)) {
        return false;
    }
    std::vector<int> networkMatrix(totalSize);
    std::transform(matrix.begin(), matrix.end(), networkMatrix.begin(), htonl);

    const char* data = reinterpret_cast<const char*>(networkMatrix.data());
    int totalBytes = totalSize * sizeof(int);
    int sentBytes = 0;

    while (sentBytes < totalBytes) {
        int toSend = std::min(chunkSize, totalBytes - sentBytes);
        int bytes = send(socket, data + sentBytes, toSend, 0);
        if (bytes <= 0) {
            return false;
        }
        sentBytes += bytes;
    }

    return true;
}

bool CommandHandler::receiveMatrixChunked(SOCKET socket, std::vector<int>& matrix) {
    int size = 0;
    if (!receiveInt(socket, size)) {
        return false;
    }
    if (size <= 0 || size > 100000000) {
        return false;
    }
    matrix.resize(size);
    int totalBytes = size * sizeof(int);
    char* buffer = reinterpret_cast<char*>(matrix.data());
    int totalReceived = 0;

    while (totalReceived < totalBytes) {
        int bytes = recv(socket, buffer + totalReceived, totalBytes - totalReceived, 0);
        if (bytes <= 0) {
            return false;
        }
        totalReceived += bytes;
    }

    for (int& value : matrix) {
        value = ntohl(value);
    }

    return true;
}

bool CommandHandler::sendStatus(SOCKET socket, const std::string& status, int progress) {
    if (!sendCommand(socket, status)) {
        return false;
    }
    if (status == "IN_PROGRESS" && progress >= 0) {
        return sendInt(socket, progress);
    }
    return true;
}

bool CommandHandler::receiveStatus(SOCKET socket, std::string& status, int& progress) {
    if (!receiveCommand(socket, status)) {
        return false;
    }
    progress = -1;
    if (status == "IN_PROGRESS") {
        if (!receiveInt(socket, progress)) {
            return false;
        }
    }
    return true;
}

