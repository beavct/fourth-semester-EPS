#pragma once
#include <arpa/inet.h>
#include <vector>
#include <string>

using namespace std;

typedef struct {
    int *socket;
    int porta;
    struct sockaddr_in clientAddress;
}threadArgumentos;

typedef struct {
    int porta;
    string IP; // não sei se ṕrecisa
    char protocolo[3];


}infosJogador;

typedef struct {
    int jogadores;
}infosPartidas;


class Server{
    private:
        vector<int> portas;
        int partidasOcorrendo;
        vector<infosPartidas> informações;

    public:
        Server(vector<int> portas);
        ~Server();
        int iniServer();
        // write arquivo de log

};