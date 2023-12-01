// g++ -Wall -pedantic -O2 -o client client.cpp
#include <bits/stdc++.h> 
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <curl/curl.h> // pegar o próprio IP
#include <pthread.h>
#include "funcAux.hpp"
#include "client.hpp"

#define BUFFER_SIZE 4096

using namespace std;

void sendHeader(int sockfd){
    char header[] = {"EP2 - PACMAN DISTRIBUÍDO"};

    write(sockfd, header, sizeof(header));
}

// Cada operação equivale à um int de 1 à 12, e ocupa 1 byte na mensagem 
// mensagem: <Operação> [1 byte] <Tamanho do próximo campo> [3 bytes] <Campo> [Tamanho informado]
// A mensagem pode ter mais de um campo dependendo da operação
int sendCommand(string clientIP, int sockfd, vector<string> comando){
    if(sizeof(comando) == 0)
        cout << "Nenhum comando mandado" << endl;

    // novo <usuario> <senha>
    if(comando[0] == "novo"){
 
        if(sizeof(comando) < 3){
            cout << "novo <usuário> <senha>" << endl;
            return 0;
        }

        uint8_t command[] = {0x01};
        uint8_t final[] = {0xce};

        uint32_t lenghtFirstArg = htonl(comando[1].size()); // tamanho do nome de usuário
        uint32_t lenghtSecondArg = htonl(comando[2].size()); // tamanho da senha

        write(sockfd, command, 1); // escreve o comando
        write(sockfd, (uint8_t*)&lenghtFirstArg, 4); // escreve o tamanho do nome de usuário
        write(sockfd, comando[1].c_str(), comando[1].size()); // escreve o nome de usuário
        write(sockfd, (uint8_t*)&lenghtSecondArg, 4); // escreve o tamanho da senha 
        write(sockfd, comando[2].c_str(), comando[2].size()); // escreve a senha
        write(sockfd, final, 1); // escreve o ponto final

        //cout << "enviou" << endl;

        // lê a resposta do servidor 
        char recvline[3];
        ssize_t n;

        n = read(sockfd, recvline, 3);
        if(n == -1){
            cerr << "erro de leitura :(" << endl;
            close(sockfd);
        }

        if(strcmp(recvline, "OK!") == 0)
            cout << "Usuário criado com sucesso" << endl;
        else    
            cout << "Ocorreu algum problema" << endl;

        return 1;
    }
    // senha <senha antiga> <senha nova>
    else if(comando[0] == "senha"){
        if(sizeof(comando) < 3){
            cout << "senha <senha antiga> <senha nova>" << endl;
            return 0;
        }

        uint8_t command[] = {0x02};
        uint8_t final[] = {0xce};

        uint32_t lenghtFirstArg = htonl(comando[1].size()); // tamanho da senha antiga
        uint32_t lenghtSecondArg = htonl(comando[2].size()); // tamanho da senha nova

        write(sockfd, command, 1); // escreve o comando
        write(sockfd, (uint8_t*)&lenghtFirstArg, 4); // escreve o tamanho da senha antiga
        write(sockfd, comando[1].c_str(), comando[1].size()); // escreve a senha antiga
        write(sockfd, (uint8_t*)&lenghtSecondArg, 4); // escreve o tamanho da senha nova
        write(sockfd, comando[2].c_str(), comando[2].size()); // escreve a senha nova
        write(sockfd, final, 1); // escreve o ponto final

        // lê a resposta do servidor 
        char recvline[3];
        ssize_t n;

        n = read(sockfd, recvline, 3);
        if(n == -1){
            cerr << "erro de leitura :(" << endl;
            close(sockfd);
        }

        if(strcmp(recvline, "OK!") == 0)
            cout << "Usuário criado com sucesso" << endl;
        else    
            cout << "Ocorreu algum problema" << endl;

        return 2;
    }
    // entra <usuario> <senha>
    else if(comando[0] == "entra"){
        if(sizeof(comando) < 3){
            cout << "entra <usuário> <senha>" << endl;
            return 0;
        }

        uint8_t command[] = {0x03};
        uint8_t final[] = {0xce};

        uint32_t lenghtFirstArg = htonl(comando[1].size()); // tamanho do usuário
        uint32_t lenghtSecondArg = htonl(comando[2].size()); // tamanho da senha

        write(sockfd, command, 1); // escreve o comando
        write(sockfd, (uint8_t*)&lenghtFirstArg, 4); // escreve o tamanho do usuário
        write(sockfd, comando[1].c_str(), comando[1].size()); // escreve o usuário
        write(sockfd, (uint8_t*)&lenghtSecondArg, 4); // escreve o tamanho da senha
        write(sockfd, comando[2].c_str(), comando[2].size()); // escreve a senha
        write(sockfd, (uint8_t*)clientIP.size(), clientIP.size()); // escreve o tamanho do IP
        write(sockfd, &clientIP, clientIP.size()); // escreve o IP
        write(sockfd, final, 1); // escreve o ponto final


        // lê a resposta do servidor 
        char recvline[3];
        ssize_t n;

        n = read(sockfd, recvline, 3);
        if(n == -1){
            cerr << "erro de leitura :(" << endl;
            close(sockfd);
        }

        if(strcmp(recvline, "OK!") == 0)
            cout << "Usuário criado com sucesso" << endl;
        else    
            cout << "Ocorreu algum problema" << endl;

        return 3;
    }
    else if(comando[0] == "lideres"){
        uint8_t command[] = {0x04};
        uint8_t final[] = {0xce};

        write(sockfd, command, 1); // escreve o comando
        write(sockfd, final, 1); // escreve o ponto final

        // lê a resposta do servidor


        return 4;
    }
    else if(comando[0] == "l"){
        uint8_t command[] = {0x05};
        uint8_t final[] = {0xce};

        write(sockfd, command, 1); // escreve o comando
        write(sockfd, final, 1); // escreve o ponto final

        char recvline[BUFFER_SIZE];
        ssize_t n;

        cout << "Lista de usuários conectados" << endl;

        uint32_t quantJogadores;
        n = read(sockfd, &quantJogadores, 4);
        if(n == -1){
            cerr << "erro de leitura :(" << endl;
            close(sockfd);
            return 0;
        }
        quantJogadores = ntohl(quantJogadores);

        uint32_t userSize;
        uint8_t jogando;
        for (int i=0; i<(int)quantJogadores; i++){
            // lê o tamanho do username
            n = read(sockfd, &userSize, 4);
            if(n == -1){
                cerr << "erro de leitura :(" << endl;
                close(sockfd);
                return 0;
            }

            userSize = ntohl(userSize);

            // lê o username
            n = read(sockfd, recvline, userSize);
            if(n == -1){
                cerr << "erro de leitura :(" << endl;
                close(sockfd);
                return 0;
            }

            // lê bool se está jogando ou não
            n = read(sockfd, &jogando, 1);
            if(n == -1){
                cerr << "erro de leitura :(" << endl;
                close(sockfd);
                return 0;
            }

            cout << "Usuário: " << recvline;
            if(jogando)
                cout << " está jogando." << endl;
            else    
                cout << " não está jogando está jogando." << endl;
        }

        return 5;
    }
    else if(comando[0] == "inicia"){
        uint8_t command[] = {0x06};
        uint8_t final[] = {0xce};

        write(sockfd, command, 1); // escreve o comando
        write(sockfd, final, 1); // escreve o ponto final

        return 6;
    }
    // desafio <oponente>
    else if(comando[0] == "desafio"){
        if(sizeof(comando) < 2){
            cout << "desafio <oponente>" << endl;
            return 0;
        }

        uint8_t command[] = {0x07};
        uint8_t final[] = {0xce};

        write(sockfd, command, 1); // escreve o comando
        write(sockfd, final, 1); // escreve o ponto final

        return 7;
    }
    // move <direcao>
    else if(comando[0] == "move"){
        if(sizeof(comando) < 2){
            cout << "move <direção>" << endl;
            return 0;
        }

        uint8_t command[] = {0x08};
        uint8_t final[] = {0xce};

        write(sockfd, command, 1); // escreve o comando
        write(sockfd, comando[1].c_str(), 1);
        write(sockfd, final, 1); // escreve o ponto final

        return 8;
    }
    else if(comando[0] == "atraso"){
        uint8_t command[] = {0x09};
        uint8_t final[] = {0xce};

        write(sockfd, command, 1); // escreve o comando
        write(sockfd, final, 1); // escreve o ponto final

        return 9;
    }
    else if(comando[0] == "encerra"){
        uint8_t command[] = {0x0A};
        uint8_t final[] = {0xce};

        write(sockfd, command, 1); // escreve o comando
        write(sockfd, final, 1); // escreve o ponto final

        return 10;
    }
    else if(comando[0] == "sai"){
        uint8_t command[] = {0x0B};
        uint8_t final[] = {0xce};

        write(sockfd, command, 1); // escreve o comando
        write(sockfd, final, 1); // escreve o ponto final

        return 11;
    }
    else if(comando[0] == "tchau"){
        uint8_t command[] = {0x0C};
        uint8_t final[] = {0xce};

        write(sockfd, command, 1); // escreve o comando
        write(sockfd, (uint8_t*)clientIP.size(), clientIP.size()); // escreve o tamanho do IP
        write(sockfd, &clientIP, clientIP.size()); // escreve o IP
        write(sockfd, final, 1); // escreve o ponto final

        return 12;
    }
    else 
        cout << "comando não reconhecido :(" << endl;
    
    return 0; // O comando não foi reconhecido
}

Client::Client(char *tipoConexao, int portaServer, char *IP){
    this->conexaoServer = tipoConexao;
    this->portaServer = portaServer;
    this->serverIP = IP;
    //this->selfIP = this->getOwnIP();
    this->selfIP = "127.0.0.1";
    this->selfPorta = this->randPorta();

    //cout << "selfPorta: " << this->selfPorta << endl;
}

Client::~Client(){
    
}

int Client::randPorta(){
    int MENOR = 1025;
    int MAIOR = 65535;

    srand(time(nullptr));
    return MENOR + (rand() % (MAIOR-MENOR));
}

// Cliente P2P tem que se comportar tanto como cliente como servidor - TCP
void Client::startServerP2P(){
    int sockfd;
    struct sockaddr_in servaddr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd == -1){
        cerr << "Erro ao criar o socket TCP." << endl;
        exit(1);
    }

    this->serverP2Psocket = sockfd;

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(this->selfPorta);

    int enableReuse = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enableReuse, sizeof(int)) < 0) {
        fprintf(stderr, "Erro no socket reuse TCP\n");
        exit(1);
    }

    // Vincula o socket TCP ao endereço
    if (bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1) {
        cerr << "Erro ao vincular o socket TCP ao endereço." << endl;
        exit(1);
    }

    // Configuração para ouvir conexões TCP
    if (listen(sockfd, 1) == -1) {
        cerr << "Erro ao configurar para ouvir conexões TCP." << endl;
        exit(1);
    }

    

    cout << "[Servidor P2P no ar. Aguardando conexões na porta " << this->selfPorta << "]" << endl;
}

// Cliente P2P tem que se comportar tanto como cliente como servidor - TCP
void Client::handle2Player(int clientSocket, string& clientIP, int clientPorta){
    char recvline[BUFFER_SIZE];
    int n;

    while(true){

        n = read(clientSocket, recvline, BUFFER_SIZE);

        if(n < 0){
            cout << "Erro de leitura :(" << endl;
        }
        recvline[n] = 0;

        cout << "Mensagem de " << clientIP << ":" << clientPorta << ":" << recvline << endl;

        if ((fputs(recvline,stdout)) == EOF) {
           perror("fputs error :(");
           exit (1);
        }

        string response = "recebi: ";
        response += recvline;
        write(clientSocket, &response, sizeof(response));
    }


    close(clientSocket);

    cout << "Conexão com " << clientIP << ":" << clientPorta <<  " fechada" << endl;
}

void Client::connect2Peer(string &peerIP, int peerPorta){
    int peerSocket;
    struct sockaddr_in peeraddr;

    if ((peerSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        cerr << "Erro ao criar o socket TCP." << endl;
        exit(1);
    }

    bzero(&peeraddr, sizeof(peeraddr));
    peeraddr.sin_family = AF_INET;
    peeraddr.sin_port = htons(peerPorta);

    if (inet_pton(AF_INET, peerIP.c_str(), &peeraddr.sin_addr) <= 0) {
        perror("inet_pton error");
        exit(1);
    }

    if (connect(peerSocket, (struct sockaddr*)&peeraddr, sizeof(peeraddr)) == -1) {
        std::cerr << "Erro ao conectar ao peer." << std::endl;
        exit(1);
    }

    std::cout << "Conectado ao peer " << peerIP << ":" << peerPorta << std::endl;

    thread([&]() {
        this->handle2Player(peerSocket, peerIP, peerPorta);
    }).detach();

}

// Funcionando
void Client::TCPconnectionServer(int porta){
	int sockfd;
	struct sockaddr_in servaddr;

	// Criação do socket TCP
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		cerr << "Erro ao criar o socket TCP." << endl;
            exit(1);
	}

    int enableReuse = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enableReuse, sizeof(int)) < 0) {
        cerr << "Erro no socket reuse TCP." << endl;
        exit(1);
    }


	bzero(&servaddr, sizeof(servaddr));
	// assign IP, PORT
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(this->serverIP.c_str());
	servaddr.sin_port = htons(porta);      

    if (inet_pton(AF_INET, serverIP.c_str(), &servaddr.sin_addr) <= 0) {
        perror("inet_pton error");
        exit(1);
    }

	// Conecta ao servidor
	if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1) {
		cerr << "Erro ao conectar com o servidor :(" << endl;
		exit(1);
	}

    cout << "[Conectando no servidor no IP " << this->serverIP << "]" << endl;

    // TESTE COMANDOS
    //sendHeader(sockfd);
    //sendCommand(this->selfIP, sockfd, vector<string>{"novo", "bea", "1234"});

    char recvline[BUFFER_SIZE];
    int n;
    while(true){
        fgets(recvline,BUFFER_SIZE,stdin);

        write(sockfd, recvline, BUFFER_SIZE);
        n = read(sockfd, recvline, BUFFER_SIZE);
        if(n == -1){
            cerr << "Erro na leitura :(" << endl;
             exit(1);
        }
        else if (n == 0) {
            // O servidor fechou a conexão
            cout << "O servidor fechou a conexão." << endl;
            break;
        }

        recvline[n] = 0;
        if ((fputs(recvline,stdout)) == EOF) {
           perror("fputs error :(");
           exit (1);
        }
    }

    close(sockfd);
}

// Código fornecido pelo Daniel
void Client::UDPconnectionServer(int porta){
    int	sockfd;
    struct  sockaddr_in servaddr;
    struct  sockaddr_in dadosLocal;
    int     dadosLocalLen,servaddrLen;
    struct  hostent *hptr;
    char    enderecoIPServidor[INET_ADDRSTRLEN];

    if ( (hptr = gethostbyname(this->serverIP.c_str())) == NULL) {
        fprintf(stderr,"gethostbyname :(\n");
        exit(1);
    }
    if (hptr->h_addrtype != AF_INET) {
        fprintf(stderr,"h_addrtype :(\n");
        exit(1);
    }
    if ( (inet_ntop(AF_INET, hptr->h_addr_list[0], enderecoIPServidor, sizeof(enderecoIPServidor))) == NULL) {
        fprintf(stderr,"inet_ntop :(\n");
        exit (1);
    }

    printf("[Conectando no servidor no IP %s]\n",enderecoIPServidor);

    /* UDP: Sockets UDP tem que ser especificados com SOCK_DGRAM */
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        cerr << "Erro ao criar o socket UDP." << endl;
            exit(1);
    }

    int enableReuse = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enableReuse, sizeof(int)) < 0) {
        fprintf(stderr, "Erro no socket reuse UDP\n");
        exit(1);
    }

    bzero(&servaddr, sizeof(servaddr));
    dadosLocalLen=sizeof(dadosLocal);
    bzero(&dadosLocal, dadosLocalLen);
    servaddr.sin_family = AF_INET;
    servaddr.sin_port   = htons(porta);

    /* UDP: É necessário passar o tamanho da estrutura para usar na função que
     * envia mensagem */
    servaddrLen=sizeof(servaddr); 

    if (inet_pton(AF_INET, enderecoIPServidor, &servaddr.sin_addr) <= 0) {
        perror("inet_pton error");
        exit(1);
    }
   
    if (getsockname(sockfd, (struct sockaddr *) &dadosLocal, (socklen_t *) &dadosLocalLen)) {
        perror("getsockname error :(");
        exit(1);
    }

    int n;
    char	recvline[BUFFER_SIZE];
    while (1)  {
        fgets(recvline,BUFFER_SIZE,stdin);

        /* UDP: Agora cada datagrama é independente pois não há
         * conexão, então a cada envio de mensagem é necessário
         * informar para onde ela vai */
        sendto(sockfd,recvline,strlen(recvline),0,(struct sockaddr *)&servaddr,(socklen_t)servaddrLen);
        /* UDP: O recebimento de mensagem também tem que ser feito
         * independente, pois não há mais conexão */
        n=recvfrom(sockfd,recvline,BUFFER_SIZE,0,(struct sockaddr *)&servaddr,(socklen_t *)&servaddrLen);
        if(n == -1){
            cerr << "Erro na leitura :(" << endl;
             exit(1);
        }
        //else if (n == 0) {
        //    // O servidor fechou a conexão
        //    cout << "O servidor fechou a conexão." << endl;
        //    break;
        //}
        
        recvline[n]=0;
        if ((fputs(recvline,stdout)) == EOF) {
           perror("fputs error :(");
           exit (1);
        }

    }
	if (n < 0) {
	    perror("read error :(");
	    exit(1);
	}
   
}

int main(int argc, char **argv){
    // Não enviou os parâmetros corretamente
    if (argc != 4) {
        fprintf(stderr,"Uso: %s <Endereço IP do servidor> <Porta do servidor> <Protocolo>\n",argv[0]);
		exit(1);
	}

    Client cliente(argv[3], atoi(argv[2]), argv[1]);

    if(strcmp(argv[3], "UDP") == 0)
        cliente.UDPconnectionServer(atoi(argv[2]));
    else if(strcmp(argv[3], "TCP") == 0)
        cliente.TCPconnectionServer(atoi(argv[2]));
    else    
        cerr << "O servidor não suporta conexões " << argv[3] << endl;

    // ----------------------------------------------------------------------------------------
    // CONTINUO DEPOIS
    //thread([&](){
    //    cliente.startServerP2P();
    //}).detach();
//
    //int porta;
    //string endereçoPadrao = "127.0.0.1";
    //cin >> porta;
//
    //cliente.connect2Peer(endereçoPadrao, porta);

    return 0;
}