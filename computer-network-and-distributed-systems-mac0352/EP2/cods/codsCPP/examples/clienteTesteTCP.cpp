// CHAT GPT
// g++ -Wall -pedantic -O2 -o clienteTesteTCP clienteTesteTCP.cpp | rm *.o
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

const int BUFFER_SIZE = 1024;

int main() {
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        std::cerr << "Erro ao criar o socket TCP do cliente." << std::endl;
        return -1;
    }

    sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1"); // Use o endereço IP do seu servidor
    serverAddress.sin_port = htons(8080); // Use a porta do seu servidor

    if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        std::cerr << "Erro ao conectar ao servidor TCP." << std::endl;
        close(clientSocket);
        return -1;
    }

    // Envia dados para o servidor
    const char* message = "Hello, TCP Server!";
    send(clientSocket, message, strlen(message), 0);

    // Recebe a resposta do servidor
    char buffer[BUFFER_SIZE];
    ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (bytesRead > 0) {
        buffer[bytesRead] = '\0';
        std::cout << "Resposta do servidor TCP: " << buffer << std::endl;
    }

    // Fecha o socket do cliente
    close(clientSocket);

    return 0;
}
