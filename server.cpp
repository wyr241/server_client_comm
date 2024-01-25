#include <iostream>
#include <fstream>
#include <cstring>
#include <ctime>
#include <vector>
#include <thread>
#include <mutex>
#include <unistd.h>
#include <arpa/inet.h>

std::mutex fileMutex;

class TCPServer {
private:
    int serverSocket;
    int serverPort;

public:
    TCPServer(int port) : serverSocket(-1), serverPort(port) {}

    ~TCPServer() {
        if (serverSocket != -1) {
            close(serverSocket);
        }
    }

    void run() {
        createServerSocket();
        bindSocket();
        listenForClients();
    }

private:
    void createServerSocket() {
        serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket == -1) {
            perror("Error creating socket");
            exit(EXIT_FAILURE);
        }
    }

    void bindSocket() {
        struct sockaddr_in serverAddr{};
        memset(&serverAddr, 0, sizeof(serverAddr));

        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(serverPort);
        serverAddr.sin_addr.s_addr = INADDR_ANY;

        if (bind(serverSocket, reinterpret_cast<struct sockaddr*>(&serverAddr), sizeof(serverAddr)) == -1) {
            perror("Error binding socket");
            exit(EXIT_FAILURE);
        }
    }

    void listenForClients() {
        if (listen(serverSocket, 5) == -1) {
            perror("Error listening for clients");
            exit(EXIT_FAILURE);
        }

        while (true) {
            int clientSocket = accept(serverSocket, nullptr, nullptr);
            if (clientSocket == -1) {
                perror("Error accepting client connection");
                exit(EXIT_FAILURE);
            }

            std::thread clientThread(&TCPServer::handleClient, this, clientSocket);
            clientThread.detach(); // Detach the thread since we don't need to join it explicitly
        }
    }

    void handleClient(int clientSocket) {
        char buffer[1024];
        ssize_t bytesRead;

        while ((bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0)) > 0) {
            buffer[bytesRead] = '\0';

            // Locking the file access with a mutex
            std::lock_guard<std::mutex> lock(fileMutex);

            // Writing the received message to log.txt
            std::ofstream logFile("log.txt", std::ios::app);
            if (logFile.is_open()) {
                logFile << buffer << std::endl;
                logFile.close();
            } else {
                std::cerr << "Error opening log file" << std::endl;
            }
        }

        close(clientSocket);
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <server_port>" << std::endl;
        return EXIT_FAILURE;
    }

    int serverPort = std::stoi(argv[1]);

    TCPServer server(serverPort);
    server.run();

    return EXIT_SUCCESS;
}
