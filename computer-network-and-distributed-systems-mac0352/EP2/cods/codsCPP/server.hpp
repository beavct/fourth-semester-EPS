#pragma once
#include <arpa/inet.h>
#include <vector>
#include <string>
#include <map>

using namespace std;

// Para as funções paralelizadas
typedef struct {
    int *socket;
    int porta;
    struct sockaddr_in clientAddress;
}threadArgumentos;

// Dados de login do usuário
typedef struct{
    string nomeUsuario;
    string senha;
}loginInfo;

// Todas as informações do cliente
typedef struct {
    int porta;
    string IP; // não sei se ṕrecisa
    string protocolo;
    int socket;
    loginInfo *login;
}infosCliente;

typedef struct {
    int jogadores; // Quantidade de jogadores na partida
    vector<map<int, char>> jogadorPersonagem; // Qual socket joga com qual jogador
}infosPartidas;


class Server{
    private:
        vector<int> portas; // Portas que o servidor está ouvindo
        vector<infosCliente> clientes; // Informações dos clientes que estão conectados
        int partidasOcorrendo;
        vector<infosPartidas> informações;

    public:
        Server(vector<int> portas);
        ~Server();
        int iniServer();
        void sendHeartBeat(int clientSocket);
        int handleCommands(int sockfd);
};
