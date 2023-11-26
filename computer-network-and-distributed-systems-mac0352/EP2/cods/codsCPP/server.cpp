#include <bits/stdc++.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include <netdb.h> 
#include <pthread.h>
#include "server.hpp"

#define NUM_THREADS 200
#define BUFFER_SIZE 4096
#define HEARTBEAT_INTERVAL 5
#define arquivoLog "log.txt"

using namespace std;

string getTime(){
    time_t currentTime = std::time(0);

    tm *localTime = localtime(&currentTime); 

    // Constrói a string de data e hora
    stringstream datetime;
    datetime << localTime->tm_year + 1900 << '-'
             << localTime->tm_mon + 1 << '-'
             << localTime->tm_mday << ' '
             << localTime->tm_hour << ':'
             << localTime->tm_min << ':'
             << localTime->tm_sec;

    // Obtém a string resultante
    return datetime.str();
}

void escreveLog(vector<string> parametros){
    // Abre o arquivo log.txt em modo de apensamento (append)
    ofstream logFile(arquivoLog, ios::app);

    if (!logFile.is_open()) {
        // Se não conseguir abrir o arquivo, exibe uma mensagem de erro
        cerr << "Erro ao abrir o arquivo de log. :(" << endl;
        return;
    }

    // Escreve o momento da ação
    logFile << getTime() << " ";

    // Escreve os parâmetros no arquivo, separados por espaço
    for (const auto& parametro : parametros) {
        logFile << parametro << " ";
    }

    // Adiciona uma nova linha ao final
    logFile << endl;

    // Fecha o arquivo
    logFile.close();

}

void* handleTCPConnection(void *arg){
    threadArgumentos *args = new threadArgumentos;
    args = (threadArgumentos*)arg;
    int clientSocket = *(int*)args->socket;
    //int porta = *(int*)args->porta;

    char recvline[BUFFER_SIZE];

    // echo
    ssize_t n;
    while((n=read(clientSocket, recvline, BUFFER_SIZE)) > 0){
        recvline[n]=0;
        if ((fputs(recvline,stdout)) == EOF) {
            perror("fputs :( \n");
            exit(6);
        }
        write(clientSocket, recvline, strlen(recvline));
    }

    // fecha a conexão com o cliente
    if(n==0){
        cout << "Fechando a conexão TCP" << endl;
        close(clientSocket);
    }
    delete(int*)arg;

    // Terminando a execução da thread
    pthread_exit(nullptr);
}

void* handleUDPConnection(void *arg){

    threadArgumentos *args = new threadArgumentos;
    args = (threadArgumentos*)arg;
    int udpSocket = *(int*)args->socket;
    //struct sockaddr_in clientAddress = (struct sockaddr_in)args->clientAddress;
    //socklen_t clientAddressLength = sizeof(clientAddress);

    struct sockaddr_in clientaddr;
    socklen_t clientaddrLen = sizeof(clientaddr);

    //int porta = *(int*)args->porta;

    char recvline[BUFFER_SIZE];

    //cout << "socket UDP: " << udpSocket << endl;

    //  UDP: Agora cada datagrama é independente pois não há
    // conexão, então a cada recebimento de mensagem é necessário
    // informar para onde ela vai e gravar de onde ela veio
    ssize_t n;
    while (1) {
        n = recvfrom(udpSocket, recvline, BUFFER_SIZE, 0, (struct sockaddr*)&clientaddr, &clientaddrLen);

        // não estou escrevendo o endereço IP no arquivo log
        escreveLog(vector<string>{"Conexão realizada por um cliente", string(inet_ntoa(clientaddr.sin_addr))});

        cout << "Endereço IP da conexão UDP: " << string(inet_ntoa(clientaddr.sin_addr)) << endl;

        if(n == -1){
            pthread_exit(nullptr);
            return nullptr;
        }
            
        // Ecoa os dados de volta para o cliente UDP
        // UDP: O envio de mensagem também tem que ser feito
        // independente, pois não há mais conexão */
        recvline[n] = 0;
        if ((fputs(recvline,stdout)) == EOF) {
            perror("fputs error");
            exit (1);
        }
        sendto(udpSocket, recvline, n, 0, (struct sockaddr*)&clientaddr, clientaddrLen);
    }

    delete (int*)arg; 

    // Termina a execução da thread
    pthread_exit(nullptr);
}

int Server::handleCommands(int sockfd){
    // Cada operação equivale à um int de 1 à 12, e ocupa 1 byte na mensagem 
    // mensagem: <Operação> [1 byte] <Tamanho do próximo campo> [4 bytes] <Campo> [Tamanho informado]
    // A mensagem pode ter mais de um campo dependendo da operação

    
}

Server::Server(vector<int> portas){
    for(int i=0; i<(int)portas.size(); i++){
        this->portas.push_back(portas[i]);
    }

    this->partidasOcorrendo = 0;   
}

Server::~Server(){
    
}

void Server::sendHeartBeat(int clientSocket){

}

int Server::iniServer(){
    vector<int> tcpSockets(this->portas.size()), udpSockets(this->portas.size());
    vector<struct sockaddr_in> servaddr(this->portas.size());

    // Criação dos sockets UDP e TCP
    for(int i = 0; i< (int)this->portas.size(); i++){
        tcpSockets[i] = socket(AF_INET, SOCK_STREAM, 0);
        udpSockets[i] = socket(AF_INET, SOCK_DGRAM, 0);

        if (tcpSockets[i] == -1) {
            cerr << "Erro ao criar o socket TCP." << endl;
            exit(1);
        }

        if (udpSockets[i] == -1) {
            cerr << "Erro ao criar o socket UDP." << endl;
            exit(1);
        }

        bzero(&servaddr[i], sizeof(servaddr[i]));
        servaddr[i].sin_family = AF_INET;
        servaddr[i].sin_addr.s_addr = htonl(INADDR_ANY);
        servaddr[i].sin_port = htons(this->portas[i]);

        ///Fiz no EP1
        int enableReuse = 1;
        if (setsockopt(tcpSockets[i], SOL_SOCKET, SO_REUSEADDR, &enableReuse, sizeof(int)) < 0) {
            fprintf(stderr, "Erro no socket reuse TCP\n");
            exit(1);
        }

        if (setsockopt(udpSockets[i], SOL_SOCKET, SO_REUSEADDR, &enableReuse, sizeof(int)) < 0) {
            fprintf(stderr, "Erro no socket reuse UDP\n");
            exit(1);
        }

        // Vincula o socket TCP ao endereço
        if (bind(tcpSockets[i], (struct sockaddr*)&servaddr[i], sizeof(servaddr[i])) == -1) {
            cerr << "Erro ao vincular o socket TCP ao endereço." << endl;
            exit(1);
        }

        // Vincula o socket UDP ao endereço
        if (bind(udpSockets[i], (struct sockaddr*)&servaddr[i], sizeof(servaddr[i])) == -1) {
            cerr << "Erro ao vincular o socket UDP ao endereço." << endl;
            exit(1);
        }

        // Configuração para ouvir conexões TCP
        if (listen(tcpSockets[i], 1) == -1) {
            cerr << "Erro ao configurar para ouvir conexões TCP." << endl;
            exit(1);
        }

        // UDP não tem que ficar escutando
    }

    // se pá tem que arrumar isso daqui
    //vector<struct sockaddr_in> clientaddr(this->portas.size());
    struct sockaddr_in clientaddr;
    socklen_t clientaddrLen;
    
    //cout<< string(inet_ntoa(servaddr[1].sin_addr)) << endl;
    //cout<< string(inet_ntoa(servaddr[0].sin_addr)) << endl;

    escreveLog(vector<string>{"Servidor iniciado"});


    cout << "[Servidor no ar. Aguardando conexões nas portas";
    for(int i=0; i<(int)this->portas.size(); i++){
        cout << " " << this->portas[i] ;
    } 
    cout << ".]" << endl;


    pthread_t tcpThreads[NUM_THREADS], udpThreads[NUM_THREADS]; // Vetor de threads de cada protocolo
    long t1 = 0; // contagem de threads TCP
    long t2 = 0; // contagem de threads UDP

    // O servidor roda enquanto ctrl-C não é pressionado
    while(cin){
        for(int i=0; i<(int)this->portas.size(); i++){

            // thread.detach() -> executa a thread em segundo plano e continua a execução do resto do código

            // Cuida das conexões TCP
            thread([&](){
                int *TCPclientSocket  = new int;

                *TCPclientSocket = accept(tcpSockets[i], (struct sockaddr*)&clientaddr, &clientaddrLen);
                
                // Escreve no arquivo log que o cliente se conectou <IP>
                escreveLog(vector<string>{"Conexão realizada por um cliente", string(inet_ntoa(clientaddr.sin_addr))});

                // Guarda as informações do cliente
                infosCliente aux;
                aux.IP = string(inet_ntoa(clientaddr.sin_addr));
                aux.porta = clientaddr.sin_port;
                aux.protocolo = "TCP";
                aux.login = nullptr;
                this->clientes.push_back(aux);

                //cout << "Endereço IP da conexão TCP: " << string(inet_ntoa(clientaddr.sin_addr)) << endl;
                

                // Argumentos para a thread do TCP
                threadArgumentos *argsTCP = new threadArgumentos;
                argsTCP->socket = TCPclientSocket;
                argsTCP->porta = this->portas[i];

                // Thread para conexão TCP
                if(*TCPclientSocket != -1){
                    if(pthread_create(&tcpThreads[t1], nullptr, handleTCPConnection, argsTCP) != -1){
                        t1++;
                    }
                    else{
                        cerr << "TCP: pthread_create :(" << endl;
                        delete TCPclientSocket;
                        delete argsTCP;
                    }
                }

            }).detach();

            // Cuida das conexões UDP
            thread([&](){
                int *UDPclientSocket = new int;
                *UDPclientSocket = udpSockets[i];

                // UDP não tem que aceitar conexão

                // Argumentos para a thread do UDP
                threadArgumentos *argsUDP = new threadArgumentos;
                //argsUDP->porta = this->portas[i];
                argsUDP->socket = UDPclientSocket;
                //argsUDP->clientAddress = servaddr[i];

                // Thread para conexão UDP
                if(pthread_create(&udpThreads[t2], nullptr, handleUDPConnection, argsUDP) != -1){
                    t2++;
                }
                else{
                    cerr << "UDP: pthread_create :(" << endl;
                    delete UDPclientSocket;
                    delete argsUDP;
                }

            }).detach();

        }
        
    }

    escreveLog(vector<string>{"Servidor finalizado"});

    for (int i = 0; i < t1; i++) {
        pthread_join(udpThreads[i], NULL);
    }
    for (int i = 0; i < t2; i++) {
        pthread_join(tcpThreads[i], NULL);
    }

    // Fecha os sockets ao finalizar 
    for (int i = 0; i < (int)this->portas.size(); i++) {
        close(tcpSockets[i]);
        close(udpSockets[i]);
    }

    return 0;
}

int main(int argc, char *argv[]){

    // A porta(s) que o servidor irá escutar não foi informada
    if(argc < 2) {
        cerr << "Uso: " << argv[0] << " <porta1> ...." << endl;
        return -1;
    }

    // Guarda a porta(s) que o servidor irá escutar
    vector<int> portas;
    for(int i=1; i<argc; i++){
        int auxPorta = stoi(argv[i]);
        portas.push_back(auxPorta);
    }

    Server servidor(portas);

    servidor.iniServer();

    return 0;
}