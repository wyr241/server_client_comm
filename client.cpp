#include <iostream>
#include <cstring>
#include <ctime>
#include <unistd.h>
#include <arpa/inet.h>

class TCPClient {
private:
    int clientSocket;
    std::string clientName;
    int serverPort;
    int connectionPeriod;

public:
    TCPClient(const std::string& name, int port, int period)
        : clientSocket(-1), clientName(name), serverPort(port), connectionPeriod(period) {}

    ~TCPClient() {
        if (clientSocket != -1) {
            close(clientSocket);
        }
    }

    void run() {
        while (true) {
            connectToServer();
            sendToServer();
            closeConnection();
            sleep(connectionPeriod);
        }
    }

private:
    void connectToServer() {
        struct sockaddr_in serverAddr{};
        memset(&serverAddr, 0, sizeof(serverAddr));

        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(serverPort);
        serverAddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

        clientSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (clientSocket == -1) {
            perror("Error creating socket");
            exit(EXIT_FAILURE);
        }

        if (connect(clientSocket, reinterpret_cast<struct sockaddr*>(&serverAddr), sizeof(serverAddr)) == -1) {
            perror("Error connecting to server");
            exit(EXIT_FAILURE);
        }
    }

    void sendToServer() {
        time_t currentTime = time(nullptr);
        tm* localTime = localtime(&currentTime);

        // char timestamp[30];

        // int millisec;
        // struct timeval tv;

        // gettimeofday(&tv, NULL);

        // millisec = lrint(tv.tv_usec/1000.0); // Round to nearest millisec
        
        // if (millisec>=1000) { // Allow for rounding up to nearest second
        //   millisec -=1000;
        //   tv.tv_sec++;
        // }

        strftime(timestamp, sizeof(timestamp), "[%Y-%m-%d %H:%M:%S.000] ", localTime);

        //printf("%s.%03d\n", buffer, millisec);
        //std::string timestring = timestamp + ".%03d(millisec)]";

        std::string message = timestamp + clientName;

        if (send(clientSocket, message.c_str(), message.length(), 0) == -1) {
            perror("Error sending message to server");
            exit(EXIT_FAILURE);
        }
    }

    void closeConnection() {
        close(clientSocket);
        clientSocket = -1;
    }
};

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <client_name> <server_port> <connection_period>" << std::endl;
        return EXIT_FAILURE;
    }

    std::string clientName = argv[1];
    int serverPort = std::stoi(argv[2]);
    int connectionPeriod = std::stoi(argv[3]);

    TCPClient client(clientName, serverPort, connectionPeriod);
    client.run();

    return EXIT_SUCCESS;
}
