#include <iostream>
#include <winsock2.h>
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

    bool startListening() {
        if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
            std::cout << "Listening socket failed: " << WSAGetLastError() << std::endl;
            closesocket(serverSocket);
            WSACleanup();
            return false;
        }
        std::cout << "Server started listening on port " << port << std::endl;

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