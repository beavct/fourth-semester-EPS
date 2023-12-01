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
#include "funcAux.hpp"
#include "server.hpp"

#define NUM_THREADS 200
#define BUFFER_SIZE 4096
#define HEARTBEAT_INTERVAL 5
#define arquivoLog "log.txt"
#define arquivoCSV "dados.csv"

vector<infosCliente> clientesInfo;

using namespace std;

// Guarda as informações de login dos clientes
void escreveCSV(string User, string password){
    // Abre o arquivo log.txt em modo de apensamento (append)
    ofstream CSVFile;

    // Verifica se o arquivo existia anteriormente
    ifstream existentCSV(arquivoCSV);
    bool alredyExists = existentCSV.good();

    CSVFile.open(arquivoCSV, ios::app);

    if (!CSVFile.is_open()) {
        // Se não conseguir abrir o arquivo, exibe uma mensagem de erro
        cerr << "Erro ao abrir o arquivo de CSV :(" << endl;
        return;
    }

    // Se o arquivo não existia anteriomente, coloca o cabeçalho  
    if(!alredyExists){
        CSVFile << "Nome de usuário; Senha" << endl;
    }

    CSVFile << User << ";" << password << endl;
}

// Retorna uma string com data e hora formatados
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

// Escreve no arquivo log
void escreveLog(vector<string> parametros){
    // Abre o arquivo log.txt em modo de apensamento (append)
    ofstream logFile(arquivoLog, ios::app);

    if (!logFile.is_open()) {
        // Se não conseguir abrir o arquivo, exibe uma mensagem de erro
        cerr << "Erro ao abrir o arquivo de log :(" << endl;
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

// OK
int readHeader(int sockfd, string protocol){
    char recvline[BUFFER_SIZE];
    ssize_t n;
    
    if(protocol == "TCP"){
        n = read(sockfd, recvline, 25);

        if(n == -1){
            cerr << "Erro de leitura :(" << endl;
            close(sockfd);
            return 0;
        }

        //cout << recvline << endl;
        
        // "EP2 - PACMAN DISTRIBUÍDO"
        if(strcmp(recvline,"\x45\x50\x32\x20\x2d\x20\x50\x41\x43\x4d\x41\x4e\x20\x44\x49\x53\x54\x52\x49\x42\x55\xc3\x8d\x44\x4f") == 0){
            return 1; // aceita a conexão do cliente
        }
        else{
            write(sockfd, "EP2 - PACMAN DISTRIBUÍDO", 25);
            close(sockfd); // fecha a conexão
            return 0; 
        }
    }
    else{ // UDP
        cout << "não implementei ainda" << endl;
    }


    return 0;
}

void aux(int sockfd){
    ssize_t n;
    char recvline[BUFFER_SIZE];
    n = read(sockfd, recvline, 68);
    if(n == -1){
        cerr << "erro de leitura :(" << endl;
        close(sockfd);
    }

    cout << recvline << endl;
}

int readCommand(int sockfd, string protocol){
    if(protocol == "TCP"){
        ssize_t n;
        char recvline[BUFFER_SIZE];
        uint8_t commandType;

        // Lê o comando
        n = read(sockfd, recvline, 1);
        if(n == -1){
            cerr << "erro de leitura :(" << endl;
            close(sockfd);
            return 0;
        }

        commandType = charToInt(&recvline[0], 1);

        // novo <usuario> <senha>
        if(commandType == 1){
            uint32_t userSize, passwordSize;

            n = read(sockfd, &userSize, sizeof(uint32_t));
            if(n == -1){
                cerr << "erro de leitura :(" << endl;
                close(sockfd);
                return 0;
            }

            userSize = ntohl(userSize);

            char *username = new char(userSize+1);

            // Lê o username
            n = read(sockfd, username, userSize);
            if(n == -1){
                cerr << "erro de leitura :(" << endl;
                close(sockfd);
                return 0;
            }

            username[userSize] = '\0';

            n = read(sockfd, &passwordSize, sizeof(uint32_t));
            if(n == -1){
                cerr << "erro de leitura :(" << endl;
                close(sockfd);
                return 0;
            }

            passwordSize = ntohl(passwordSize);

            char *password = new char (passwordSize+1);

            // Lê a senha
            n = read(sockfd, password, passwordSize);
            if(n == -1){
                cerr << "erro de leitura :(" << endl;
                close(sockfd);
                return 0;
            }     

            password[passwordSize] = '\0';

            // lê o ponto final
            n = read(sockfd, recvline, 1);
            if(n == -1){
                cerr << "erro de leitura :(" << endl;
                close(sockfd);
                return 0;
            }   

            // Guarda as informações de login do cliente
            for(int i=0; i<(int)clientesInfo.size(); i++){
                if(clientesInfo[i].socket == sockfd){
                    clientesInfo[i].login->nomeUsuario = username;
                    clientesInfo[i].login->senha = password;
                }
            }

            write(sockfd, "OK!", 3);

            // Deixa esse login salvo no CSV
            //escreveCSV(username, password);
        }
        // senha <senha antiga> <senha nova>
        else if(commandType == 2){
            uint32_t oldPassSize, newPassSize;

            // lê o tamanho da senha antiga
            n = read(sockfd, &oldPassSize, sizeof(uint32_t));
            if(n == -1){
                cerr << "erro de leitura :(" << endl;
                close(sockfd);
                return 0;
            }

            oldPassSize = ntohl(oldPassSize);

            char *oldPass = new char (oldPassSize+1);

            // lê a senha antiga
            n = read(sockfd, oldPass, oldPassSize);
            if(n == -1){
                cerr << "erro de leitura :(" << endl;
                close(sockfd);
                return 0;
            }

            oldPass[oldPassSize] = '\0';

            // lê o tamanho da senha nova
            n = read(sockfd, &newPassSize, sizeof(uint32_t));
            if(n == -1){
                cerr << "erro de leitura :(" << endl;
                close(sockfd);
                return 0;
            }

            newPassSize = ntohl(newPassSize);

            char *newPass = new char (newPassSize+1);

            // lê a senha nova
            n = read(sockfd, newPass, newPassSize);
            if(n == -1){
                cerr << "erro de leitura :(" << endl;
                close(sockfd);
                return 0;
            }

            newPass[newPassSize] = '\0';

            // lê o ponto final
            n = read(sockfd, recvline, 1);
            if(n == -1){
                cerr << "erro de leitura :(" << endl;
                close(sockfd);
                return 0;
            }   

            // troca a senha
            for(int i=0; i<(int)clientesInfo.size(); i++){
                if(clientesInfo[i].socket == sockfd){
                    if(clientesInfo[i].login->senha == oldPass){
                        // troca se senha bem sucedida
                        clientesInfo[i].login->senha = newPass;
                        write(sockfd, "OK!", 3);
                    }
                    else{
                        // troca de senha não ocorreu, pois o usuário errou a senha antiga
                        write(sockfd, "OPS", 3);
                    }

                    break;
                }
            }
            
        }
        // entra <usuario> <senha>
        else if(commandType == 3){
            uint32_t userSize, passwordSize, ipSize;

            n = read(sockfd, &userSize, sizeof(uint32_t));
            if(n == -1){
                cerr << "erro de leitura :(" << endl;
                close(sockfd);
                return 0;
            }

            userSize = ntohl(userSize);

            char *username = new char(userSize+1);

            // Lê o username
            n = read(sockfd, username, userSize);
            if(n == -1){
                cerr << "erro de leitura :(" << endl;
                close(sockfd);
                return 0;
            }

            username[userSize] = '\0';

            n = read(sockfd, &passwordSize, sizeof(uint32_t));
            if(n == -1){
                cerr << "erro de leitura :(" << endl;
                close(sockfd);
                return 0;
            }

            passwordSize = ntohl(passwordSize);

            char *password = new char (passwordSize+1);

            // Lê a senha
            n = read(sockfd, password, passwordSize);
            if(n == -1){
                cerr << "erro de leitura :(" << endl;
                close(sockfd);
                return 0;
            }     

            password[passwordSize] = '\0';

            n = read(sockfd, &ipSize, sizeof(uint32_t));
            if(n == -1){
                cerr << "erro de leitura :(" << endl;
                close(sockfd);
                return 0;
            }

            ipSize = ntohl(ipSize);

            char *ipCliente = new char (ipSize+1);

            ipCliente[ipSize] = '\0';

            // Lê a senha
            n = read(sockfd, ipCliente, ipSize);
            if(n == -1){
                cerr << "erro de leitura :(" << endl;
                close(sockfd);
                return 0;
            }     

            // lê o ponto final
            n = read(sockfd, recvline, 1);
            if(n == -1){
                cerr << "erro de leitura :(" << endl;
                close(sockfd);
                return 0;
            }   

            int achou = 0;

            // procura nas informações dos usuários
            for(int i=0; i<(int)clientesInfo.size(); i++){
                if(clientesInfo[i].login->nomeUsuario == username && clientesInfo[i].login->senha == password && !clientesInfo[i].logado){
                    // conseguiu logar
                    write(sockfd, "OK!", 3);
                    clientesInfo[i].logado = 1;
                    achou = 1;

                    escreveLog(vector<string>{"Login com sucesso", username, ipCliente});
                }
            }   

            if(!achou){
                // não conseguiu logar
                write(sockfd, "OPS", 3);
                escreveLog(vector<string>{"Login sem sucesso", username, ipCliente});
            }         
            
        }
        // lideres
        else if(commandType == 4){
            
        }
        // l
        else if(commandType == 5){
            //<quantidade de jogadores> <tamanho do nome> <nome> <se está jogando ou não, 0 ou 1> ....

            int cont = 0;

            for(int i=0; i<(int)clientesInfo.size(); i++){
                if(clientesInfo[i].logado)
                    cont++;
            }

            // quantidade de jogares
            uint32_t quantJogadores = htonl(cont);
            write(sockfd, (uint8_t*)&quantJogadores, sizeof(uint32_t));

            uint32_t userSize;
            for(int i=0; i<(int)clientesInfo.size(); i++){
                if(clientesInfo[i].logado){
                    userSize = htonl(clientesInfo[i].login->nomeUsuario.size());
                    
                    // tamanho do user do jogador i, o user do jogador i, se está jogando ou não
                    write(sockfd, (uint8_t*)&userSize, sizeof(uint32_t));
                    write(sockfd, &clientesInfo[i].login->nomeUsuario, clientesInfo[i].login->nomeUsuario.size());
                    write(sockfd, &clientesInfo[i].jogando, 1);
                }
            }
        }
        // inicia
        else if(commandType == 6){
            // inicia uma partida, o cliente é o pacman
        }
        // desafio <oponente>
        else if(commandType == 7){
            // entra na partida de alguem, como fantasma (só pode um fantasma "f")
        }
        // move <direção>
        else if(commandType == 8){
            
        }
        // atraso
        else if(commandType == 9){
            // durante uma partida com outro oponente, informa os 3 últimos valores de latência que
            // foram medidos para esse outro oponente. Se não tiver oponente, não precisa retornar nada.
        }
        // encerra
        else if(commandType == 10){
            // encerra uma partida antes da hora
        }
        // sai
        else if(commandType == 11){
            // desloga
        }
        // tchau
        else if(commandType == 12){
            uint32_t ipSize, recvline;

            n = read(sockfd, &ipSize, sizeof(uint32_t));
            if(n == -1){
                cerr << "erro de leitura :(" << endl;
                close(sockfd);
                return 0;
            }

            ipSize = ntohl(ipSize);

            char *ipCliente = new char (ipSize+1);

            ipCliente[ipSize] = '\0';

            // Lê a senha
            n = read(sockfd, ipCliente, ipSize);
            if(n == -1){
                cerr << "erro de leitura :(" << endl;
                close(sockfd);
                return 0;
            }     

            // lê o ponto final
            n = read(sockfd, &recvline, 1);
            if(n == -1){
                cerr << "erro de leitura :(" << endl;
                close(sockfd);
                return 0;
            }   

            escreveLog(vector<string>{"Desconexão realizada por um cliente", ipCliente});

            // finaliza a execução do cliente e retorna para o shell do sistema operacional
            close(sockfd);
        }


    }
    else{ // UDP

    }

    return 0;
}

void* handleTCPConnection(void *arg){
    threadArgumentos *args = new threadArgumentos;
    args = (threadArgumentos*)arg;
    int clientSocket = *(int*)args->socket;
    //int porta = *(int*)args->porta;

    // TESTE LEITURA DO COMANDO
    //if(readHeader(clientSocket, "TCP")){
    //    cout << "indo pro readCommand" << endl;
    //    readCommand(clientSocket, "TCP"); 
    //}

    // ------------------------------------------------------
    // SERVIDOR ECHO
    char recvline[BUFFER_SIZE];
    ssize_t n;
    while(true){
        n=read(clientSocket, recvline, BUFFER_SIZE);
        if(n == -1){
            close(clientSocket);
            pthread_exit(nullptr);
            return nullptr;
        }

        recvline[n]=0;
        if ((fputs(recvline,stdout)) == EOF) {
            perror("fputs :( \n");
            exit(6);
        }
        write(clientSocket, recvline, strlen(recvline));
    }

    // fecha a conexão com o cliente
    //if(n==0){
    //    cout << "Fechando a conexão TCP" << endl;
    //    close(clientSocket);
    //}
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
    int flag = 0;
    while (1) {
        n = recvfrom(udpSocket, recvline, BUFFER_SIZE, 0, (struct sockaddr*)&clientaddr, &clientaddrLen);

        if(!flag){
            // guarda os dados do cliente
            infosCliente aux;
            aux.IP = string(inet_ntoa(clientaddr.sin_addr));
            aux.porta = clientaddr.sin_port;
            aux.protocolo = "UDP";
            aux.login = nullptr;
            aux.logado = 0;

            clientesInfo.push_back(aux);

            // não estou escrevendo o endereço IP no arquivo log
            escreveLog(vector<string>{"Conexão realizada por um cliente", string(inet_ntoa(clientaddr.sin_addr))});
            flag = 1;
        }

        //cout << "Endereço IP da conexão UDP: " << string(inet_ntoa(clientaddr.sin_addr)) << endl;

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
                

                // Guarda as informações do cliente
                infosCliente aux;
                aux.IP = string(inet_ntoa(clientaddr.sin_addr));
                aux.porta = clientaddr.sin_port;
                aux.protocolo = "TCP";
                aux.login = nullptr;
                aux.logado = 0;

                clientesInfo.push_back(aux);
                //this->clientes.push_back(aux);

                //cout << "Endereço IP da conexão TCP: " << string(inet_ntoa(clientaddr.sin_addr)) << endl;
                

                // Argumentos para a thread do TCP
                threadArgumentos *argsTCP = new threadArgumentos;
                argsTCP->socket = TCPclientSocket;
                argsTCP->porta = this->portas[i];

                // Thread para conexão TCP
                if(*TCPclientSocket != -1){

                    // Escreve no arquivo log que o cliente se conectou <IP>
                    escreveLog(vector<string>{"Conexão realizada por um cliente", string(inet_ntoa(clientaddr.sin_addr))});

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