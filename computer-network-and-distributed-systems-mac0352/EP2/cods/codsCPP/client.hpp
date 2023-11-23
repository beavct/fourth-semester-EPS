#pragma once
#include <arpa/inet.h>
#include <vector>
#include <string>

using namespace std;

class Client{
    private:
<<<<<<< HEAD
        string conexaoServer; // QUal o ripo de conexão que vai fazer com o servidor
        int portaServer; // Em qual porta vai se conectar do servidor
        string serverIP; // IP do servidor que quer se conectar
        string selfIP; // Seu próprio IP
        int selfPorta; // Porta que vai rodar o servidor P2P
        int serverP2Psocket; // Socket de servidor P2P
=======
        string conexaoServer;
        int portaServer;
        string serverIP;
        string selfIP;
>>>>>>> 48c989c651ad8e7af41ea3bd589485918557ede4

    public:
        Client(char *tipoConexao, int portaServer, char *IP);
        ~Client();
        void connect2Server(); // TCP e UDP
<<<<<<< HEAD
        void TCPconnectionServer(int porta);
        void UDPconnectionServer(int porta);
        void startServerP2P();
        void handle2Player(int clientSocket, string& clientIP, int clientPorta); // TCP p2p
        int randPorta(); // Define a porta que vai rodar o servidor P2P
        void connect2Peer(string &peerIP, int peerPorta);
=======
        void TCPconnection(int porta);
        void UDPconnection(int porta);
        void connect2Player(); // TCP

>>>>>>> 48c989c651ad8e7af41ea3bd589485918557ede4

};