#include <iostream>
#include <winsock2.h>
#include "MatrixOperations/MatrixOperations.h"
#include "CommandHandler/CommandHandler.h"
#pragma comment(lib, "ws2_32.lib")

class Client {
private:
    SOCKET clientSocket;
    sockaddr_in serverAddress;
    int port = 8080;
    const char* serverIp = "127.0.0.1";

public:
    bool initializeConnection() {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cout << "WSAStartup failed\n";
            return false;
        }
        clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (clientSocket == INVALID_SOCKET) {
            std::cout << "Error creating socket: " << WSAGetLastError() << "\n";
            WSACleanup();
            return false;
        }
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_addr.s_addr = inet_addr(serverIp);
        serverAddress.sin_port = htons(port);
        if (connect(clientSocket, reinterpret_cast<sockaddr*>(&serverAddress), sizeof(serverAddress)) == SOCKET_ERROR) {
            std::cout << "Connect failed with error: " << WSAGetLastError() << "\n";
            closesocket(clientSocket);
            WSACleanup();
            return false;
        }
        std::cout << "Client connected to server successfully\n";

        sockaddr_in localAddress;
        int addrLength = sizeof(localAddress);
        if (getsockname(clientSocket, (sockaddr*)&localAddress, &addrLength) == 0) {
            std::cout << "Client address: " << inet_ntoa(localAddress.sin_addr) << ":" << ntohs(localAddress.sin_port) << std::endl;
        }

        return true;
    }

    void startClient() {
        int matrixSize, threadCount;
        std::cout << "Enter matrix size: ";
        std::cin >> matrixSize;
        std::cout << "Enter number of threads: ";
        std::cin >> threadCount;

        int* matrix = MatrixOperations::allocateMatrix(matrixSize);
        MatrixOperations::fillMatrix(matrix, matrixSize);

        std::cout << "Connecting to the server...\n";

        CommandHandler::sendCommand(clientSocket, "CONFIG");
        CommandHandler::sendInt(clientSocket, threadCount);
        std::string configResponse;
        CommandHandler::receiveCommand(clientSocket, configResponse);
        if (configResponse != "CONFIG_OK") {
            std::cout << "Error in configuration.\n";
            MatrixOperations::freeMatrix(matrix);
            return;
        }
        std::cout << "Server confirmed configuration.\n";

        CommandHandler::sendCommand(clientSocket, "MATRIX_SIZE");
        CommandHandler::sendInt(clientSocket, matrixSize);
        std::string sizeResponse;
        CommandHandler::receiveCommand(clientSocket, sizeResponse);

        if (sizeResponse != "SIZE_OK") {
            std::cout << "Error sending matrix size to server.\n";
            MatrixOperations::freeMatrix(matrix);
            return;
        }
        std::cout << "Matrix size accepted by server.\n";
    }
};

int main() {
    Client client;
    if (!client.initializeConnection()) {
        std::cout << "Failed to initialize client connection.\n";
        return false;
    }
    client.startClient();

    return 0;
}