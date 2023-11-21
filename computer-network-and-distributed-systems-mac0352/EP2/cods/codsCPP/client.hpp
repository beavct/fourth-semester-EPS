#pragma once
#include <arpa/inet.h>
#include <vector>
#include <string>

using namespace std;

class Client{
    private:
        string conexaoServer;
        int portaServer;
        string serverIP;
        string selfIP;

    public:
        Client(char *tipoConexao, int portaServer, char *IP);
        ~Client();
        void connect2Server(); // TCP e UDP
        void TCPconnection(int porta);
        void UDPconnection(int porta);
        void connect2Player(); // TCP


};