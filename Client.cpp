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

        std::vector<int> vec(matrix, matrix + matrixSize * matrixSize);
        CommandHandler::sendCommand(clientSocket, "DATA");

        if (!CommandHandler::sendMatrixChunked(clientSocket, vec, 4096)) {
            std::cout << "Error sending matrix data to server.\n";
            MatrixOperations::freeMatrix(matrix);
            return;
        }

        std::string dataResponse;
        CommandHandler::receiveCommand(clientSocket, dataResponse);

        if (dataResponse != "DATA_OK") {
            std::cout << "Server did not accept matrix data.\n";
            MatrixOperations::freeMatrix(matrix);
            return;
        }
        std::cout << "Matrix data sent successfully.\n";

        if (matrixSize <= 10) {
            std::cout << "\nInitial Matrix:\n";
            MatrixOperations::printMatrix(matrix, matrixSize);
        }

        CommandHandler::sendCommand(clientSocket, "START");
        std::string startResponse;
        CommandHandler::receiveCommand(clientSocket, startResponse);

        if (startResponse != "STARTED") {
            std::cout << "Error starting processing on server.\n";
            MatrixOperations::freeMatrix(matrix);
            return;
        }
        std::cout << "Server started processing matrix.\n";

        while (true) {
            CommandHandler::sendCommand(clientSocket, "STATUS");

            std::string status;
            int progress = -1;
            bool statusReceived  = CommandHandler::receiveStatus(clientSocket, status, progress);

            if (!statusReceived ) {
                std::cout << "Error receiving status.\n";
                break;
            }

            if (status == "IN_PROGRESS") {
                std::cout << "Processing: " << progress << "% complete.\n";
            }
            else if (status == "DONE") {
                std::cout << "Processing done by server.\n";
                break;
            }
            else {
                std::cout << "Unknown status received: " << status << "\n";
                break;
            }

            Sleep(100);
        }

        CommandHandler::sendCommand(clientSocket, "RESULT");
        std::vector<int> resultMatrix;

        bool resultReceived = CommandHandler::receiveMatrixChunked(clientSocket, resultMatrix);
        if (!resultReceived || resultMatrix.empty()) {
            std::cout << "Error receiving matrix result.\n";
            MatrixOperations::freeMatrix(matrix);
            return;
        }

        std::cout << "Processed matrix received from server.\n";

        CommandHandler::sendCommand(clientSocket, "END");
        closesocket(clientSocket);
        WSACleanup();

        if (matrixSize <= 10) {
            std::cout << "\nProcessed Matrix:\n";
            MatrixOperations::printMatrix(resultMatrix.data(), matrixSize);
        }

        MatrixOperations::freeMatrix(matrix);
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