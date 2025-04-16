#include <iostream>
#include <winsock2.h>
#include <thread>
#include <vector>
#include "MatrixOperations/MatrixOperations.h"
#include "CommandHandler/CommandHandler.h"
#pragma comment(lib,"ws2_32.lib")

class Server {
private:
    SOCKET serverSocket;
    int port = 8080;
    bool serverRunning  = true;

public:
    bool initializeServer() {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cout << "WSAStartup failed" << std::endl;
            return false;
        }
        serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (serverSocket == INVALID_SOCKET) {
            std::cout << "Socket creation failed: " << WSAGetLastError() << std::endl;
            WSACleanup();
            return false;
        }
        sockaddr_in serverAddress;
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_addr.s_addr = INADDR_ANY;
        serverAddress.sin_port = htons(port);
        if (bind(serverSocket, reinterpret_cast<sockaddr*>(&serverAddress), sizeof(serverAddress)) == SOCKET_ERROR) {
            std::cout << "Socket binding failed: " << WSAGetLastError() << std::endl;
            closesocket(serverSocket);
            WSACleanup();
            return false;
        }
        return true;
    }

    void handleClient(SOCKET clientSocket, sockaddr_in clientAddress) {
        std::vector<int> matrix;
        int matrixSize = 0;
        int threadsCount = 0;
        bool processing = false;
        bool isRunning = true;

        std::atomic<int> columnsProcessed(0);

        while (isRunning) {
            std::string command;
            if (!CommandHandler::receiveCommand(clientSocket, command)) {
                break;
            }

            if (command == "CONFIG") {
                if (!CommandHandler::receiveInt(clientSocket, threadsCount)) {
                    break;
                }
                CommandHandler::sendCommand(clientSocket, "CONFIG_OK");
                std::cout << "Configuration accepted. Threads: " << threadsCount << std::endl;
            }
            else if (command == "MATRIX_SIZE") {
                if (!CommandHandler::receiveInt(clientSocket, matrixSize)) {
                    break;
                }
                CommandHandler::sendCommand(clientSocket, "SIZE_OK");
                std::cout << "Matrix size received: " << matrixSize << "x" << matrixSize << std::endl;
            }
            else if (command == "DATA") {
                std::vector<int> receivedMatrix;
                if (!CommandHandler::receiveMatrixChunked(clientSocket, receivedMatrix)) {
                    CommandHandler::sendCommand(clientSocket, "DATA_ERR");
                    std::cout << "Invalid matrix data." << std::endl;
                }
                else {
                    matrix = receivedMatrix;
                    CommandHandler::sendCommand(clientSocket, "DATA_OK");
                    std::cout << "Matrix data received." << std::endl;
                }
            }
            else if (command == "START") {
                if (matrix.empty() || matrixSize <= 0 || threadsCount <= 0) {
                    CommandHandler::sendCommand(clientSocket, "START_ERR");
                    std::cout << "Failed to start: check matrix size and number of threads." << std::endl;
                }
                else {
                    processing = true;
                    columnsProcessed = 0;
                    CommandHandler::sendCommand(clientSocket, "STARTED");

                    std::thread processingThread([&]() {
                        MatrixOperations::calculateMaxMultiThread(matrix.data(), matrixSize, threadsCount, &columnsProcessed);
                        processing = false;
                    });
                    processingThread.detach();
                    std::cout << "Computation started." << std::endl;
                }
            }
            else if (command == "STATUS") {
                int progress = (columnsProcessed * 100) / matrixSize;
                if (processing && progress < 100) {
                    CommandHandler::sendStatus(clientSocket, "IN_PROGRESS", progress);
                }
                else {
                    CommandHandler::sendStatus(clientSocket, "DONE");
                }
            }
            else if (command == "RESULT") {
                if (!processing) {
                    CommandHandler::sendMatrixChunked(clientSocket, matrix, 4096);
                    std::cout << "Processed matrix sent to client." << std::endl;
                }
                else {
                    CommandHandler::sendCommand(clientSocket, "RESULT_ERR");
                }
            }
            else if (command == "END") {
                isRunning = false;
                std::cout << "Session ended by client " << inet_ntoa(clientAddress.sin_addr) << ":" << ntohs(clientAddress.sin_port) << "." << std::endl;
            }
            else {
                CommandHandler::sendCommand(clientSocket, "UNKNOWN_COMMAND");
            }
        }

        closesocket(clientSocket);
    }

    bool startListening() {
        if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
            std::cout << "Listening socket failed: " << WSAGetLastError() << std::endl;
            closesocket(serverSocket);
            WSACleanup();
            return false;
        }
        std::cout << "Server started listening on port " << port << std::endl;
        while (serverRunning) {
            sockaddr_in clientAddress;
            int clientAddressSize = sizeof(clientAddress);
            SOCKET clientSocket = accept(serverSocket, reinterpret_cast<sockaddr*>(&clientAddress), &clientAddressSize);
            if (clientSocket == INVALID_SOCKET) {
                std::cout << "Client accept failed: " << WSAGetLastError() << std::endl;
                continue;
            }
            char* clientIp = inet_ntoa(clientAddress.sin_addr);
            int clientPort = ntohs(clientAddress.sin_port);
            std::cout << "New client connected: IP - " << clientIp << ", port - " << clientPort << std::endl;
            std::thread(&Server::handleClient, this, clientSocket, clientAddress).detach();

        }
        closesocket(serverSocket);
        WSACleanup();
        return true;
    }
};

int main() {
    Server server;
    if (!server.initializeServer()) {
        std::cout << "Failed to initialize server." << std::endl;
        return false;
    }
    if (!server.startListening()) {
        std::cout << "Failed to start listening." << std::endl;
        return false;
    }

    return 0;
}