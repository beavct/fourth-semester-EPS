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
#include "client.hpp"

#define BUFFER_SIZE 4096

using namespace std;

Client::Client(char *tipoConexao, int portaServer, char *IP){
    this->conexaoServer = tipoConexao;
    this->portaServer = portaServer;
    this->serverIP = IP;
    //this->selfIP = IP;
}

Client::~Client(){
    
}

void Client::connect2Server(){

}

void Client::connect2Player(){

}

// não está funcionando
void Client::TCPconnection(int porta){
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
        recvline[n] = 0;
        if ((fputs(recvline,stdout)) == EOF) {
           perror("fputs error :(");
           exit (1);
        }
    }

    close(sockfd);
}

// Copiei descaradamente do código do Daniel
void Client::UDPconnection(int porta){
    int	sockfd;
    struct  sockaddr_in servaddr;
    struct  sockaddr_in dadosLocal;
    int     dadosLocalLen,servaddrLen;
    struct  hostent *hptr;
    char    enderecoIPServidor[INET_ADDRSTRLEN];

    // Copiei na cara dura do daniel
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
    printf("[o comando 'exit' encerra a conexão]\n");

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
        /* write(sockfd, recvline, strlen(recvline)); */
        sendto(sockfd,recvline,strlen(recvline),0,(struct sockaddr *)&servaddr,(socklen_t)servaddrLen);
        /* UDP: O recebimento de mensagem também tem que ser feito
         * independente, pois não há mais conexão */
        /* n=read(sockfd,recvline,MAXLINE); */
        n=recvfrom(sockfd,recvline,BUFFER_SIZE,0,(struct sockaddr *)&servaddr,(socklen_t *)&servaddrLen);
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
        cliente.UDPconnection(atoi(argv[2]));
    else if(strcmp(argv[3], "TCP") == 0)
        cliente.TCPconnection(atoi(argv[2]));
    else    
        cerr << "O servidor não suporta conexões " << argv[3] << endl;


    return 0;
}