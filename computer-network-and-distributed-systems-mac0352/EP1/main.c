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
#include <signal.h>

/*BIBLIOTECAS NOVAS*/
#include <pthread.h>
#include "extraFunctions.h"

#define LISTENQ 1
#define MAXDATASIZE 100
#define NUM_THREADS 200
#define MAX_BUFFER_SIZE 4096

/*VARIÁVEIS GLOBAIS*/
queueList *globalList;

void *makeConnection(void *arg);

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

    int enableReuse = 1;

    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &enableReuse, sizeof(int)) < 0) {
        fprintf(stderr, "Erro no socket reuse\n");
        exit(1);
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
   

    /*Inicializa a lista de filas*/
   globalList = inicializateQueue();

	for (;;) {

        if ((connfd = accept(listenfd, (struct sockaddr *) NULL, NULL)) == -1 ) {
            perror("accept :(\n");
            exit(5);
        }

        /*Estrutura para a utilização de Pthreads*/
        struct ThreadArgs *args = (struct ThreadArgs *)malloc(sizeof(struct ThreadArgs));
        if (args == NULL) {
            perror("malloc :(\n");
            exit(6);
        }
        /*Assim conseguiremos ter acesso aos sockets dentro da rotina com Pthreads*/
        args->connfd = connfd;
        args->globalList = globalList;

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


    }

    for (int i = 0; i < t; i++) {
        pthread_join(threads[i], NULL);
    }

    close(listenfd);

    exit(0);
}

/*Função paralelizada*/
void *makeConnection(void *arg){
    /*Necessário por conta das threads*/
    struct ThreadArgs *args = (struct ThreadArgs *)arg;
    int connfd = args->connfd;
    queueList *globalList = args->globalList;

    /*Leitura do pacote e identificação do método*/
    char request[MAX_BUFFER_SIZE];
    int methodID;
    ssize_t n;
    int consumer = 0;
    uint8_t **retC = NULL;

    if(readHeader(connfd)){

            /*ínicio da conexão e abertura do canal*/
            Connection_Start(connfd);
            Connection_Tune(connfd);
            Connection_Open(connfd);
            Channel_Open(connfd);

            n = read(connfd, request, 7);
            if(n <= 0){
                close(connfd);
                return NULL;
            }

            int length = charToInt(&request[3], 4);

            n = read(connfd, request+7, length + 1);

            //n = read(connfd, request, MAX_BUFFER_SIZE);
            if(n <= 0){
                close(connfd);
                return NULL;
            }

            methodID = charToInt(&request[9], 2);

            /*Criação de fila - OK*/
            if(methodID == 10){
                uint8_t* ret;

                /*Declara a fila*/
                ret = Queue_Declare(connfd, request);

                globalList = reallocQueue(globalList, ret);

                /*Fecha a conexão*/
                Channel_Close(connfd);
                Connection_Close(connfd);
            }
            /*Inscrever consumidor*/
            else if(methodID == 20){
                consumer = 1;

                retC = Basic_Consume(globalList, connfd, request);

                /*Significa que a fila solicitada não foi encontrada*/
                if(retC == NULL){
                    close(connfd);
                    return NULL;
                }
                
                reallocSocket(globalList, connfd, retC[0], retC[1]);

            }
            /*Publicar mensagem*/
            else if(methodID == 40){
                Basic_Publish(globalList, connfd, request);
                Channel_Close(connfd);
                Connection_Close(connfd);
                Basic_Ack(connfd);
            }

    }

    /*Se for um consumidor ele fica esperando chegar mensagens até apertar Ctrl-C.
    Se não, apenas fecha a conexão.*/
    if(!consumer){
            /*Fecha a conexão do socket*/
            close(connfd);
    }
    else{
        int n;
        int recvline[MAX_BUFFER_SIZE];

        /*Espera o consumer dar Ctrl-C*/
        while((n = read(connfd, recvline, MAX_BUFFER_SIZE))){
        }

        /*Remove o socket do consumer da lista de sockets da fila especifica*/
        if(n ==0)
            removeConsumer(globalList, connfd, retC[0]);
    }

    free(args);

    return NULL;
}
