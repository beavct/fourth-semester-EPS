/* Por Prof. Daniel Batista <batista@ime.usp.br>
 * Em 27/8/2023
 * 
 * Um código simples de um servidor de eco a ser usado como base para
 * o EP1. Ele recebe uma linha de um cliente e devolve a mesma linha.
 * Teste ele assim depois de compilar:
 * 
 * ./redes-servidor-exemplo-ep1 8000
 * 
 * Com este comando o servidor ficará escutando por conexões na porta
 * 8000 TCP (Se você quiser fazer o servidor escutar em uma porta
 * menor que 1024 você precisará ser root ou ter as permissões
 * necessáfias para rodar o código com 'sudo').
 *
 * Depois conecte no servidor via telnet. Rode em outro terminal:
 * 
 * telnet 127.0.0.1 8000, 127.0.0.1 é o servidor
 * 
 * Escreva sequências de caracteres seguidas de ENTER. Você verá que o
 * telnet exibe a mesma linha em seguida. Esta repetição da linha é
 * enviada pelo servidor. O servidor também exibe no terminal onde ele
 * estiver rodando as linhas enviadas pelos clientes.
 * 
 * Obs.: Você pode conectar no servidor remotamente também. Basta
 * saber o endereço IP remoto da máquina onde o servidor está rodando
 * e não pode haver nenhum firewall no meio do caminho bloqueando
 * conexões na porta escolhida.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>
#include <unistd.h>


/*BIBLIOTECAS NOVAS*/
#include <pthread.h>

#define LISTENQ 1
#define MAXDATASIZE 100
#define MAXLINE 4096


/*DEFINES NOVOS*/
#define MAX_BUFFER_SIZE 1024
#define NUM_THREADS 150

/*VARIÁVEIS GLOBAIS*/
uint8_t deliveryTag = 1;


/*STRUCTS NOVAS*/
// Estrutura de argumentos para as threads
struct ThreadArgs {
    int connfd;
};


/*FUNÇÕES NOVAS*/

/*Para testes*/
void printaHex(uint8_t* msg, int size){
    int i;

    for(i=0; i<size+21; i++)
        printf("0x%X ", msg[i]);
}

int charToInt(char* msg, int size) {
    int valor = 0;
    for (int i = 0; i < size; i++) {
        /*shift de 8 bits*/
        valor = valor << 8;
        valor += msg[i];
    }

    return valor;
}

/*Gerador de Consumer-Tag aleatorizado*/
uint8_t* generateConsumerTag(){
    int i;
    int size = rand() % 32;

    uint8_t* consumerTag = (uint8_t*)malloc(sizeof(uint8_t)*size);

    srand(time(NULL));
    for(i = 0; i < size; i++){
        /*255 é o max de uint8_t*/
        consumerTag[i] = 65 + (rand() % (122 - 65));
    }

    return consumerTag;
}

/*Se retornar 1 - Pode prosseguir para o Conection.Start, c.c. a conexão é recusada e o programa retorna a header correta*/
int readHeader(int connfd){
    /*A header que vamos ler tem 8 bytes*/
    char header[8];
    ssize_t n;

    n = read(connfd, header, 8);

    /*Envia a header de volta para o cliente*/

    /*Verifica se o cliente está pedindo uma conexão com a versão AMQP 0.9.1*/
    if(n <= 0 || strncmp("\x41\x4d\x51\x50\x00\x00\x09\x01", header, 8) != 0){
        /*Retorna a header correta e fecha a conexão com o socket*/
        write(connfd, "\x41\x4d\x51\x50\x00\x00\x09\x01", 8);
        close(connfd);
        return 0;
    }

    return 1;
}   

/*No final arrumar aq - tem a assinatura do RabbitMQ*/
/*Requisição de Conexão*/
void Connection_Start(int connfd){
    char request[MAX_BUFFER_SIZE];
    ssize_t n;

    /*HEX retirado do Wireshark, Connection.Start-OK pelo servidor, src.port==5672*/
    write(connfd, "\x01\x00\x00\x00\x00\x02\x00\x00\x0a\x00\x0a\x00\x09\x00\x00\x01" \
        "\xdb\x0c\x63\x61\x70\x61\x62\x69\x6c\x69\x74\x69\x65\x73\x46\x00" \
        "\x00\x00\xc7\x12\x70\x75\x62\x6c\x69\x73\x68\x65\x72\x5f\x63\x6f" \
        "\x6e\x66\x69\x72\x6d\x73\x74\x01\x1a\x65\x78\x63\x68\x61\x6e\x67" \
        "\x65\x5f\x65\x78\x63\x68\x61\x6e\x67\x65\x5f\x62\x69\x6e\x64\x69" \
        "\x6e\x67\x73\x74\x01\x0a\x62\x61\x73\x69\x63\x2e\x6e\x61\x63\x6b" \
        "\x74\x01\x16\x63\x6f\x6e\x73\x75\x6d\x65\x72\x5f\x63\x61\x6e\x63" \
        "\x65\x6c\x5f\x6e\x6f\x74\x69\x66\x79\x74\x01\x12\x63\x6f\x6e\x6e" \
        "\x65\x63\x74\x69\x6f\x6e\x2e\x62\x6c\x6f\x63\x6b\x65\x64\x74\x01" \
        "\x13\x63\x6f\x6e\x73\x75\x6d\x65\x72\x5f\x70\x72\x69\x6f\x72\x69" \
        "\x74\x69\x65\x73\x74\x01\x1c\x61\x75\x74\x68\x65\x6e\x74\x69\x63" \
        "\x61\x74\x69\x6f\x6e\x5f\x66\x61\x69\x6c\x75\x72\x65\x5f\x63\x6c" \
        "\x6f\x73\x65\x74\x01\x10\x70\x65\x72\x5f\x63\x6f\x6e\x73\x75\x6d" \
        "\x65\x72\x5f\x71\x6f\x73\x74\x01\x0f\x64\x69\x72\x65\x63\x74\x5f" \
        "\x72\x65\x70\x6c\x79\x5f\x74\x6f\x74\x01\x0c\x63\x6c\x75\x73\x74" \
        "\x65\x72\x5f\x6e\x61\x6d\x65\x53\x00\x00\x00\x19\x72\x61\x62\x62" \
        "\x69\x74\x40\x72\x6f\x64\x72\x69\x67\x6f\x2d\x56\x69\x72\x74\x75" \
        "\x61\x6c\x42\x6f\x78\x09\x63\x6f\x70\x79\x72\x69\x67\x68\x74\x53" \
        "\x00\x00\x00\x37\x43\x6f\x70\x79\x72\x69\x67\x68\x74\x20\x28\x63" \
        "\x29\x20\x32\x30\x30\x37\x2d\x32\x30\x32\x32\x20\x56\x4d\x77\x61" \
        "\x72\x65\x2c\x20\x49\x6e\x63\x2e\x20\x6f\x72\x20\x69\x74\x73\x20" \
        "\x61\x66\x66\x69\x6c\x69\x61\x74\x65\x73\x2e\x0b\x69\x6e\x66\x6f" \
        "\x72\x6d\x61\x74\x69\x6f\x6e\x53\x00\x00\x00\x39\x4c\x69\x63\x65" \
        "\x6e\x73\x65\x64\x20\x75\x6e\x64\x65\x72\x20\x74\x68\x65\x20\x4d" \
        "\x50\x4c\x20\x32\x2e\x30\x2e\x20\x57\x65\x62\x73\x69\x74\x65\x3a" \
        "\x20\x68\x74\x74\x70\x73\x3a\x2f\x2f\x72\x61\x62\x62\x69\x74\x6d" \
        "\x71\x2e\x63\x6f\x6d\x08\x70\x6c\x61\x74\x66\x6f\x72\x6d\x53\x00" \
        "\x00\x00\x11\x45\x72\x6c\x61\x6e\x67\x2f\x4f\x54\x50\x20\x32\x34" \
        "\x2e\x32\x2e\x31\x07\x70\x72\x6f\x64\x75\x63\x74\x53\x00\x00\x00" \
        "\x08\x52\x61\x62\x62\x69\x74\x4d\x51\x07\x76\x65\x72\x73\x69\x6f" \
        "\x6e\x53\x00\x00\x00\x06\x33\x2e\x39\x2e\x31\x33\x00\x00\x00\x0e" \
        "\x50\x4c\x41\x49\x4e\x20\x41\x4d\x51\x50\x4c\x41\x49\x4e\x00\x00" \
        "\x00\x05\x65\x6e\x5f\x55\x53\xce", 520);
    /*write(connfd, "0x00, 0x00, 0x00, <tamanho 4 bytes>, 0x00, 0x0A, 0x00, 0x0B", )*/

    /*write(connfd, "0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x0a, 0x00, 0x0a", 65);*/

    /*Connection.Start pelo cliente*/
    n = read(connfd, request, MAX_BUFFER_SIZE);   
    if(n <= 0){
        close(connfd);
        return;
    }     

}

/*Descrição da documentação: propose/negotiate connection tuning parameters*/
void Connection_Tune(int connfd){
    char request[MAX_BUFFER_SIZE];
    ssize_t n;

    /*Responde Connection.Tune-OK do servidor*/
    write(connfd, "\x01\x00\x00\x00\x00\x00\x0c\x00\x0a\x00\x1e\x07\xff\x00\x02\x00" \
            "\x00\x00\x3c\xce", 20);

    /*recebe Connection.Tune do cliente*/
    n = read(connfd, request, MAX_BUFFER_SIZE);
    if(n <= 0){
        close(connfd);
        return;
    }
}

/*Descrição da documentação: This method signals to the client that the connection is ready for use.*/
void Connection_Open(int connfd){
    char request[MAX_BUFFER_SIZE];
    ssize_t n;

    /*Connection.Open-OK por parte do servidor*/
    write(connfd, "\x01\x00\x00\x00\x00\x00\x05\x00\x0a\x00\x29\x00\xce", 13);

    /*Lê o Connection.Open enviado pelo cliente*/
    n = read(connfd, request, MAX_BUFFER_SIZE);
    if(n <= 0){
        close(connfd);
        return;
    }

}

void Channel_Open(int connfd){
    char request[MAX_BUFFER_SIZE];
    ssize_t n;

    /*Channel.Open-OK por parte do servidor*/
    write(connfd, "\x01\x00\x01\x00\x00\x00\x08\x00\x14\x00\x0b\x00\x00\x00\x00\xce", 16);

    /*Channel.Open por parte do cliente*/
    n = read(connfd, request, MAX_BUFFER_SIZE);
    if(n <= 0){
        close(connfd);
        return;
    }
}

void Channel_Close(int connfd){
    char request[MAX_BUFFER_SIZE];
    ssize_t n;

    /*Channel.Close por parte do cliente*/
    n = read(connfd, request, MAX_BUFFER_SIZE);    
    if(n <= 0){
        close(connfd);
        return;
    }

    /*Servidor manda Channel.Close-OK*/
    write(connfd, "\x01\x00\x01\x00\x00\x00\x04\x00\x14\x00\x29\xce", 12);

}

void Connection_Close(int connfd){
    char request[MAX_BUFFER_SIZE];
    ssize_t n;

    /*Connection.Close do cliente*/
    n = read(connfd, request, MAX_BUFFER_SIZE);
    if(n <= 0){
        close(connfd);
        return;
    }

    /*Servidor manda Connection.Close-OK*/
    write(connfd, "\x01\x00\x00\x00\x00\x00\x04\x00\x0a\x00\x33\xce", 12);

}

uint8_t* Queue_Declare(int connfd, char* request){

   /*Utilizadas para identificar as informações da fila do cliente*/ 
    int size, i;
    uint8_t *queueName;
    /*Quarda o valor de retorno*/
    uint8_t* ret;

    /*Casa do pacote recebido do cliente em que fica o tamanho do nome da fila*/
    size = charToInt(&(request[13]), 1);

    queueName = (uint8_t*)malloc(sizeof(uint8_t)*(size));

    /*Pegando o nome da fila no pacote mandado pelo cliente*/
    for(i=0; i<size; i++){
        queueName[i] = (uint8_t)request[14+i];
    }
    
    /*Construindo a resposta do servidor*/

    /*Irão ser utilizadas para construir a reposta do servidor*/
    uint8_t totalResponseSize;
    /*Tamanho do pacote (13) + tamanho do nome da fila (size) + tamanho da fila (1)*/
    totalResponseSize = 13 + size;

    /*tamanho = 12*/
    uint8_t serverResponse1[] = {0x01, 0x00, 0x01, 0x00, 0x00, 0x00, totalResponseSize, 0x00, 0x32, 0x00, 0x0b, (uint8_t)size};
    /*tamanh0 = 9
    message-count(long) e consumer-count(long)*/
    uint8_t serverResponse2[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xce};
    uint8_t *serverResponse;

    serverResponse = (uint8_t*)malloc(sizeof(uint8_t)*(size+21));

    /*Concatena tudo na resposta do servidor*/
    memcpy(serverResponse, serverResponse1, sizeof(serverResponse1));
    memcpy(serverResponse+sizeof(serverResponse1), queueName, size);
    memcpy(serverResponse+sizeof(serverResponse1)+size, serverResponse2, sizeof(serverResponse2));

    /*Mandando a resposta do servidor*/
    write(connfd, serverResponse, size+21);

    free(serverResponse);

    /*Constrói o vetor de retorno
    Tamanho do nome da fila + nome da fila + quantidade de clientes(0)*/
    ret = (uint8_t*)malloc(sizeof(uint8_t)*(size+2));
    ret[0] = (uint8_t)size;
    memcpy(ret+1, queueName, size);
    ret[size+1] = (uint8_t)0;

    free(queueName);

    return ret;
}

/*This method delivers a message to the client, via a consumer.*/
void Basic_Deliver(int connfd, uint8_t* consumerTag){
    uint8_t length = 5 + sizeof(consumerTag) + 9 + 1;
    uint8_t serverResponse1[] = {0x01, 0x00, 0x01, 0x00, 0x00, 0x00, length, 0x00, 0x3c, 0x00, 0x3c, (uint8_t)sizeof(consumerTag)};
    uint8_t serverResponse2[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, deliveryTag, 0x00};
    uint8_t *serverResponse = (uint8_t*)malloc(sizeof(uint8_t)*(21+sizeof(consumerTag)));

    deliveryTag++;

    /*Construindo a resposta do servidor*/
    memcpy(serverResponse, serverResponse1, sizeof(serverResponse1));
    memcpy(serverResponse+sizeof(serverResponse1), consumerTag, (uint8_t)sizeof(consumerTag));
    memcpy(serverResponse+sizeof(serverResponse1)+sizeof(consumerTag), serverResponse2, sizeof(serverResponse2));

    /*Mandando a resposta do servidor*/
    write(connfd, serverResponse, 21+sizeof(consumerTag));

    free(serverResponse);

}

/*Função do cliente*/
void Basic_Consume(int connfd, char* request){

    /*Utilizado para pegar a consumer-tag*/
    int queueNameSize, consumerTagSize, i;
    uint8_t* consumerTag;

    queueNameSize = charToInt(&(request[13]), 1);
    consumerTagSize = charToInt(&(request[14+queueNameSize]), 1);

    if(consumerTagSize == 0){
        consumerTag = generateConsumerTag();
        consumerTagSize = sizeof(consumerTag);
    }
    else{
        consumerTag = (uint8_t*)malloc(sizeof(uint8_t)*consumerTagSize);
        for(i=0; i<consumerTagSize; i++){
            consumerTag[i] = (uint8_t)request[15+queueNameSize+i];
        }
    }

    /*Para a resposta do servidor*/
    uint8_t length = (uint8_t)consumerTagSize + 5;
    uint8_t serverResponse1[] = {0x01, 0x00, 0x01, 0x00, 0x00, 0x00, length, 0x00, 0x3c, 0x00, 0x15, (uint8_t)consumerTagSize};
    uint8_t serverResponse2[] = {0xce};
    uint8_t *serverResponse;

    /*Construindo a resposta do servidor*/
    serverResponse = (uint8_t*)malloc(sizeof(uint8_t)*(13+consumerTagSize));
    memcpy(serverResponse, serverResponse1, sizeof(serverResponse1));
    memcpy(serverResponse+sizeof(serverResponse1), consumerTag, consumerTagSize);
    memcpy(serverResponse+sizeof(serverResponse1)+consumerTagSize, serverResponse2, 1);

    /*Mandando a resposta do servidor*/
    write(connfd, serverResponse, 13+consumerTagSize);

    Basic_Deliver(connfd, consumerTag);
}

void Basic_Publish(int connfd, char* request){
    printf("entrou no publish\n");
}

void Basic_Ack(int connfd);

/*Função paralelizada*/
void *makeConnection(void *arg){
    /*Necessário por conta das threads*/
    struct ThreadArgs *args = (struct ThreadArgs *)arg;
    int connfd = args->connfd;

    /*Leitura do pacote e identificação do método*/
    char request[MAX_BUFFER_SIZE];
    int methodID;
    ssize_t n;

    /*Filas
    FAÇO ESSA BOMBA DPS*/
    /*uint8_t* queues = NULL;
    uint8_t retAux[8];*/

    printf("[Uma conexão aberta]\n");

    if(readHeader(connfd)){

            /*ínicio da conexão e abertura do canal*/
            Connection_Start(connfd);
            Connection_Tune(connfd);
            Connection_Open(connfd);
            Channel_Open(connfd);

            printf("chegou no read\n");

            /*Leitura da frame toda*/
            /*ERRO - A vezes da merda aqui e não sai*/
            n = read(connfd, request, MAX_BUFFER_SIZE);
            if(n <= 0){
                close(connfd);
                return NULL;
            }

            printf("saiu do read\n");

            methodID = charToInt(&request[9], 2);

            printf("%d\n", methodID);

            /*Criação de fila*/
            if(methodID == 10){
                uint8_t* ret;

                /*Declara a fila*/
                ret = Queue_Declare(connfd, request);
                /*printaHex(ret, sizeof(ret));*/

                /*Fecha a conexão*/
                Channel_Close(connfd);
                Connection_Close(connfd);

            }
            /*Inscrever consumidor*/
            else if(methodID == 20) 
                Basic_Consume(connfd, request);
            /*Publicar mensagem*/
            else if(methodID == 40)
                Basic_Publish(connfd, request);


    }


    /*Fecha a conexão do socket*/
    printf("[Uma conexão fechada]\n");

    close(connfd);

    free(args);

    return NULL;
}


int main (int argc, char **argv) {
    int listenfd, connfd;
    struct sockaddr_in servaddr;
    pid_t childpid;
   
    /*VARIÁVEIS NOVAS*/
    pthread_t threads[NUM_THREADS];
    long t = 0;


    if (argc != 2) {
        fprintf(stderr,"Uso: %s <Porta>\n",argv[0]);
        fprintf(stderr,"Vai rodar um servidor de echo na porta <Porta> TCP\n");
        exit(1);
    }

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket :(\n");
        exit(2);
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port        = htons(atoi(argv[1]));
    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
        perror("bind :(\n");
        exit(3);
    }


    if (listen(listenfd, LISTENQ) == -1) {
        perror("listen :(\n");
        exit(4);
    }

    printf("[Servidor no ar. Aguardando conexões na porta %s]\n",argv[1]);
    printf("[Para finalizar, pressione CTRL+c ou rode um kill ou killall]\n");
   
	for (;;) {

        if ((connfd = accept(listenfd, (struct sockaddr *) NULL, NULL)) == -1 ) {
            perror("accept :(\n");
            exit(5);
        }

        /*Estrutura para a utilização de Pthreads*/
        struct ThreadArgs *args = (struct ThreadArgs *)malloc(sizeof(struct ThreadArgs));
        if (args == NULL) {
            perror("malloc :(\n");
            close(connfd);
            continue;
        }
        /*Assim conseguiremos ter acesso aos sockets dentro da rotina com Pthreads*/
        args->connfd = connfd;

        /*pthread_create retorna 0 quando a thread é criada corretamente*/
        if ( (childpid = pthread_create(&threads[t], NULL, makeConnection, args)) == 0) {
            
            /*Incrementa a quantidade de threads*/
            t++;
            /*exit(0);*/
        }
        else{
            printf("Não foi possível criar a thread :(\n");
            close(connfd);
        }

        /*Vai liberando as threads conforme as conexões vão fechando*/
        /*for (int i = 0; i < t; i++) {
            pthread_join(threads[i], NULL);
        }*/

    }

    close(listenfd);

    exit(0);
}
