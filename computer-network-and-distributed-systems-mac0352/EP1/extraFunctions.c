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

/*INCLUDES NOVOS*/
#include "extraFunctions.h"

/*DEFINES NOVOS*/
#define MAX_BUFFER_SIZE 4096
#define htonll(x) ((((uint64_t)htonl(x)) << 32) + htonl((x) >> 32))

/*VARIÁVEIS GLOBAIS*/
uint64_t deliveryTag = 1;

/*FUNÇÕES NOVAS*/

/*Para testes*/
void printaHex(uint8_t* msg, int size){
    int i;

    for(i=0; i<size+21; i++)
        printf("0x%X ", msg[i]);

    printf("\n");
}

/*Para testes das filas*/
void printaListas(queueList *q){
    queueNode* aux = q->head;
    socketNode* auxS;

    printf("----------------------------\n");

    printf("Tamanho da lista: %d\n", q->queueSize);

    while(aux != NULL){
        printf("Fila: %s\n", aux->queueName);
        auxS = aux->socketHead;

        printf("Quantidade de sockets: %d\n", aux->socketSize);

        if(auxS != NULL){
            printf("Socket: %d\n", auxS->connfd);
            auxS = auxS->next;
        }

        while(auxS != NULL){
            if(auxS->head)
                break;
            printf("Socket: %d\n", auxS->connfd);
            auxS = auxS->next;
        }
        aux = aux->next;
    }
    printf("----------------------------\n");
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

uint64_t charToLongLong(char* msg, int size){
    uint64_t valor = 0;
    for (int i = 0; i < size; i++) {
        /*shift de 8 bits*/
        valor = valor << 8;
        valor += msg[i];
    }

    return valor;    
}

/*Remove um consumer quando ele pressiona Ctrl-C no terminal*/
void removeConsumer(queueList *globalList, int connfd, uint8_t* queueName){
    close(connfd);
    queueNode *qAux = globalList->head;

    /*Percorre a estrutura de filas para encontrar a fila que tem que remover o consumidor*/
    while(qAux != NULL){
        if(strcmp((const char*)qAux->queueName, (const char*)queueName)==0){
            int sAux = qAux->socketSize;
            socketNode *socketAux = qAux->socketHead;

            /*Procura o consumidor na lista de filas*/
            for(int i=0; i<sAux; i++){
                if(socketAux->connfd == connfd){
                    /*De fato remove o consumidor da lista ligada circular*/
                    if(sAux == 1)
                        qAux->socketHead = NULL;
                    else {
                        socketNode *socketAux1 = socketAux->ant;
                        socketNode *socketAux2 = socketAux->next;

                        socketAux1->next = socketAux2;
                        socketAux2->ant = socketAux1;
                        
                        /*Se o nó que estamos removendo for head, temos que atualizar a head*/
                        if(socketAux->head){
                            socketAux2->head = 1;
                            qAux->socketHead = socketAux2;
                        }
                    }

                    qAux->socketSize--;    

                    break;
                }

                socketAux = socketAux->next;
            }
        }

        qAux = qAux->next;
    }

}

/*Inicializa a lista de filas*/
queueList* inicializateQueue(){
    queueList* q = (queueList*)malloc(sizeof(queueList));
    q->head = NULL;
    q->queueSize = 0;

    return q;
}

/*Insere uma nova fila na lista de filas depois de um amqp-queue-declare*/
queueList* reallocQueue(queueList* q, uint8_t *queueName){
    queueNode* aux = q->head;
    queueNode* aux2 = NULL;

    /*Enquanto ainda não olhou todos os nomes de fila*/
    while(aux != NULL){
        /*se já existe a fila, retorna a lista atual sem atualizar*/
        if(aux->queueName == queueName)
            return q;

        /*Atualizamos o ponteiro para olhar o resto da lista*/
        aux2 = aux;
        aux = aux->next;
    }

    /*Se a fila não existe, criar um nó para ela*/
    aux = (queueNode*)malloc(sizeof(queueNode));

    /*Se a lista não tem nenhum nó, esse nó é o primeiro*/
    if(aux2 == NULL)
        q->head = aux;
    /*Se não liga esse novo nó ao resto da lista de filas*/
    else
        aux2->next = aux;

    aux->queueName = queueName;
    aux->deliveryTag = htonll(deliveryTag);
    aux->next = NULL;
    aux->socketSize = 0;

    aux->socketHead = NULL;

    q->queueSize++;

    deliveryTag++;

    return q;
}

/*Verifica se a fila requisitada no consume ou publish realmente existe*/
int findQueue(queueList *q, uint8_t *queueName){
    queueNode *aux = q->head;

    while(aux != NULL){
        /*A fila foi declarada anteriormente e foi encontrada*/
        if(strcmp((const char*)aux->queueName, (const char*)queueName) == 0)
            return 1;

        aux = aux->next;
    }

    /*A fila não foi declarada anteriormente*/
    return 0;
}

/*Inscreve um socket na fila de sockets de uma lista depois de um amqp-consume*/
void reallocSocket(queueList* q, int socket, uint8_t *queueName, uint8_t* consumerTag){
    queueNode* aux = q->head;

    /*Enquanto ainda não olhou todos os nomes de fila*/
    while(aux != NULL){
        /*Se encontrou a fila sai do laço para poder inscrever o consumer nela*/
        if(strcmp((const char*)aux->queueName, (const char*)queueName) == 0)
            break;

        /*Se não, continua procurando a lista correta para inscrever o consumer*/
        aux = aux->next;
    }    

    /*Se a fila não tem nenhum socket inscrito*/
    if(aux->socketSize == 0){
        aux->socketHead = (socketNode*)malloc(sizeof(socketNode));
        aux->socketHead->connfd = socket;
        //aux->socketHead->next = NULL;
        //aux->socketHead->ant = NULL;
        aux->socketHead->next = aux->socketHead;
        aux->socketHead->ant = aux->socketHead;
        aux->socketHead->consumerTag = consumerTag;
        aux->socketSize++;
        aux->socketHead->head = 1;

        return;
    }

    socketNode *auxS = aux->socketHead;

    /*Se a fila já tem sockets inscritos, inscreve esse novo no fim da fila*/
    socketNode *auxS2;
    auxS2 = (socketNode*)malloc(sizeof(socketNode));
    auxS2->connfd = socket;
    auxS2->consumerTag = consumerTag;
    auxS2->head = 0;

    socketNode *auxAnt = auxS->ant;
    auxAnt->next = auxS2;
    auxS2->ant = auxAnt;
    auxS2->next = auxS;
    auxS->ant = auxS2;

    aux->socketSize++;

}

/*Gerador de Consumer-Tag aleatorizado*/
uint8_t* generateConsumerTag(){
    int i;
    int size = rand() % 32;

    uint8_t* consumerTag = (uint8_t*)malloc(sizeof(uint8_t)*size);

    srand(time(NULL));
    for(i = 0; i < size; i++){
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

    /*Verifica se o cliente está pedindo uma conexão com a versão AMQP 0.9.1*/
    if(n <= 0 || strncmp("\x41\x4d\x51\x50\x00\x00\x09\x01", header, 8) != 0){
        /*Retorna a header correta e fecha a conexão com o socket*/
        write(connfd, "\x41\x4d\x51\x50\x00\x00\x09\x01", 8);
        close(connfd);
        return 0;
    }

    return 1;
}   

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

    /*Connection.Start pelo cliente*/
    n = read(connfd, request, 7);
    if(n <= 0){
        close(connfd);
        return;
    }

    int length = charToInt(&request[3],4);

    n = read(connfd, request+7, length+1);
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
    n = read(connfd, request, 7);
    if(n <= 0){
        close(connfd);
        return;
    }

    int length = charToInt(&request[3],4);

    n = read(connfd, request+7, length+1);
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
    n = read(connfd, request, 7);
    if(n <= 0){
        close(connfd);
        return;
    }

    int length = charToInt(&request[3],4);

    n = read(connfd, request+7, length+1);
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
    //n = read(connfd, request, MAX_BUFFER_SIZE);
    n = read(connfd, request, 7);
    if(n <= 0){
        close(connfd);
        return;
    }

    int length = charToInt(&request[3],4);

    n = read(connfd, request+7, length+1);
    if(n <= 0){
        close(connfd);
        return;
    }
}

void Channel_Close(int connfd){
    char request[MAX_BUFFER_SIZE];
    ssize_t n;

    /*Channel.Close por parte do cliente*/
    n = read(connfd, request, 7);
    if(n <= 0){
        close(connfd);
        return;
    }

    int length = charToInt(&request[3],4);

    n = read(connfd, request+7, length+1);
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
    n = read(connfd, request, 7);
    if(n <= 0){
        close(connfd);
        return;
    }

    int length = charToInt(&request[3],4);

    n = read(connfd, request+7, length+1);
    if(n <= 0){
        close(connfd);
        return;
    }

    /*Servidor manda Connection.Close-OK*/
    write(connfd, "\x01\x00\x00\x00\x00\x00\x04\x00\x0a\x00\x33\xce", 12);

}

uint8_t* Queue_Declare(int connfd, char* request){

   /*Utilizadas para identificar as informações da fila do cliente*/ 
    int queueNameSize, i;
    uint8_t *queueName;
    /*Quarda o valor de retorno*/
    uint8_t *ret;

    /*Casa do pacote recebido do cliente em que fica o tamanho do nome da fila*/
    queueNameSize = charToInt(&(request[13]), 1);

    queueName = (uint8_t*)malloc(sizeof(uint8_t)*(queueNameSize));

    /*Pegando o nome da fila no pacote mandado pelo cliente*/
    for(i=0; i<queueNameSize; i++){
        queueName[i] = (uint8_t)request[14+i];
    }
    
    /*Construindo a resposta do servidor*/

    /*Irão ser utilizadas para construir a reposta do servidor*/
    uint32_t length;
    /*class(2) + method(2) + queueNameSize + queueName + messageCount(4) + consumerCount(4)*/
    length = 13 + queueNameSize;
    length = htonl(length);

    uint8_t type[] = {0x01};
    uint8_t channel[] = {0x00, 0x01};
    uint8_t classs[] = {0x00, 0x32};
    uint8_t method[] = {0x00, 0x0b};
    uint8_t sizeQ[] = {(uint8_t)queueNameSize};
    uint8_t messageCount[] = {0x00, 0x00, 0x00, 0x00};
    uint8_t consumerCount[] = {0x00, 0x00, 0x00, 0x00};
    uint8_t end[] = {0xce};

    write(connfd, type, 1);
    write(connfd, channel, 2);
    write(connfd, (uint8_t*)&length, 4);
    write(connfd, classs, 2);
    write(connfd, method, 2);
    write(connfd, sizeQ, 1);
    write(connfd, queueName, (uint8_t)queueNameSize);
    write(connfd, messageCount, 4);
    write(connfd, consumerCount, 4);
    write(connfd, end, 1);

    /*Constrói o array de retorno
    Tamanho do nome da fila + nome da fila + quantidade de clientes(0)*/
    ret = (uint8_t*)malloc(sizeof(uint8_t)*(queueNameSize));
    memcpy(ret, queueName, queueNameSize);

    free(queueName);

    return ret;
}

/*This method delivers a message to the client, via a consumer.*/
void Basic_Deliver(queueList *globalList, uint8_t *routingKey, uint8_t *payload, uint64_t bodySize){
    queueNode *aux = globalList->head;

    while(aux != NULL){
        if(strcmp((const char*)aux->queueName, (const char*)routingKey) == 0)
            break;

        aux = aux->next;
    }

    /*Não encontrou a fila na lista de filas ou a fila não tem sockets cadastrados, então só sai e fecha a conexão*/
    if(aux ==  NULL || aux->socketSize == 0)
        return;

    int consumerSocket = aux->socketHead->connfd;
    uint8_t *consumerTag = aux->socketHead->consumerTag;
    uint64_t deliveryTagQ = aux->deliveryTag;
    aux->socketHead->head = 0;
    aux->socketHead = aux->socketHead->next;
    aux->socketHead->head = 1;


    uint32_t length;
    /*class(2) + method(2) + queueNameSize + queueName + messageCount(4) + consumerCount(4)*/
    length = 16 + (uint8_t)sizeof(consumerTag) + (uint8_t)sizeof(routingKey);
    length = htonl(length);

    /*Envia a primeira frame*/

    uint8_t type1[] = {0x01};
    uint8_t channel1[] = {0x00, 0x01};
    uint8_t classs1[] = {0x00, 0x3c};
    uint8_t method1[] = {0x00, 0x3c};
    uint8_t sizeC[] = {(uint8_t)sizeof(consumerTag)};
    uint8_t redelivered[] = {0x00};
    uint8_t exchangeSize[] = {0x00};
    uint8_t routingKeySize[] = {(uint8_t)sizeof(routingKey)};
    uint8_t end[] = {0xce};

    write(consumerSocket, type1, 1);
    write(consumerSocket, channel1, 2);
    write(consumerSocket, (uint8_t*)&length, 4);
    write(consumerSocket, classs1, 2);
    write(consumerSocket, method1, 2);
    write(consumerSocket, sizeC, 1);
    write(consumerSocket, consumerTag, (int)sizeof(consumerTag));
    write(consumerSocket, (uint8_t*)&deliveryTagQ, 8);
    write(consumerSocket, redelivered, 1);
    write(consumerSocket, exchangeSize, 1);
    write(consumerSocket, routingKeySize, 1);
    write(consumerSocket, routingKey, (uint8_t)sizeof(routingKey));
    write(consumerSocket, end, 1);

    /*Envia o segundo frame - content-header*/
    length = (uint32_t)15;
    length = htonl(length);
    uint8_t type2[] = {0x02};
    uint8_t channel2[] = {0x00, 0x01};
    uint8_t classID[] = {0x00, 0x3c};
    uint8_t weight[] = {0x00, 0x00};
    uint8_t propertyFlags[] = {0x10, 0x00};
    /*delivery-mode não persistente*/
    uint8_t deliveryMode[] = {0x01};

    bodySize = htonll(bodySize);


    write(consumerSocket, type2, 1);
    write(consumerSocket, channel2, 2);
    write(consumerSocket, (uint8_t*)&length, 4);
    write(consumerSocket, classID, 2);
    write(consumerSocket, weight, 2);
    write(consumerSocket, (uint8_t*)&bodySize, 8);
    write(consumerSocket, propertyFlags, 2);
    write(consumerSocket, deliveryMode, 1);
    write(consumerSocket, end, 1);


    /*Envia a terceira frame - content-body*/
    length = (uint32_t)htonll(bodySize);
    length = htonl(length);
    uint8_t type3[] = {0x03};
    uint8_t channel3[] = {0x00, 0x01};

    write(consumerSocket, type3, 1);
    write(consumerSocket, channel3, 2);
    write(consumerSocket, (uint8_t*)&length, 4);
    write(consumerSocket, payload, htonll(bodySize));
    write(consumerSocket, end, 1);
}

/*Confirmação de que o cliente recebeu as mensagens e as processou com sucesso*/
int Basic_Ack(int connfd){
    char request[MAX_BUFFER_SIZE];
    ssize_t n;

    /*Basic.Ack por parte do cliente*/
    n = read(connfd, request, 7);
    if(n <= 0)
        return 0;

    int length = charToInt(&request[3], 4);

    n = read(connfd, request+7, length+1);
    if(n <= 0)
        return 0;

    return 1;
}

/*Função do consumidor da mensagem*/
uint8_t** Basic_Consume(queueList *globalList, int connfd, char* request){

    /*Utilizado para pegar a consumer-tag*/
    int queueNameSize, consumerTagSize, i;
    uint8_t *consumerTag, *queueName, **ret;

    ret = (uint8_t**)malloc(sizeof(uint8_t*)*2);

    queueNameSize = charToInt(&(request[13]), 1);
    consumerTagSize = charToInt(&(request[14+queueNameSize]), 1);

    queueName = (uint8_t*)malloc(sizeof(uint8_t)*queueNameSize);
    for(i=0; i<queueNameSize; i++){
        queueName[i] = (uint8_t)request[14+i];
    }

    /*Verifica se a fila solicitada foi declarada anteriormente*/
    int success = findQueue(globalList, queueName);
    if(!success){
        /*Resposta para o cliente*/
        uint32_t lengthE = 47 + queueNameSize;
        lengthE = htonl(lengthE);

        uint8_t typeE[] = {0x01};
        uint8_t channelE[] = {0x00, 0x01};
        uint8_t classE[] = {0x00, 0x14};
        uint8_t methodE[] = {0x00, 0x28};
        /*Reply-Code: 404*/
        uint8_t replyCodeE[] = {0x01, 0x94};
        uint8_t strSize = 22+14+queueNameSize;
        /*str: Reply-Text: NOT_FOUND - no queue */
        uint8_t replyTextE1[] = {0x4e, 0x4f, 0x54, 0x5f, 0x46, 0x4f, 0x55, 0x4e, 0x44, 0x20, 0x2d, 0x20, 0x6e, 0x6f, 0x20, 0x71, 0x75, 0x65, 0x75, 0x65, 0x20, 0x27};
        /*str: in vhost '/'*/
        uint8_t replyTextE2[] = {0x27, 0x20, 0x69, 0x6e, 0x20, 0x76, 0x68, 0x6f, 0x73, 0x74, 0x20, 0x27, 0x2f, 0x27};
        uint8_t classIdE[] = {0x00, 0x3c};
        uint8_t methodIdE[] = {0x00, 0x14};
        uint8_t endE[] = {0xce}; 

        write(connfd, typeE, 1);
        write(connfd, channelE, 2);
        write(connfd, (uint8_t*)&lengthE, 4);
        write(connfd, classE, 2);
        write(connfd, methodE, 2);
        write(connfd, replyCodeE, 2);
        write(connfd, &strSize, 1);
        write(connfd, replyTextE1, 22);
        write(connfd, queueName, queueNameSize);
        write(connfd, replyTextE2, 14);
        write(connfd, classIdE, 2);
        write(connfd, methodIdE, 2);
        write(connfd, endE, 1);

        return NULL;
    }

    if(consumerTagSize == 0){
        consumerTag = generateConsumerTag();
        consumerTagSize = (uint8_t)sizeof(consumerTag);
    }
    else{
        consumerTag = (uint8_t*)malloc(sizeof(uint8_t)*consumerTagSize);
        for(i=0; i<consumerTagSize; i++){
            consumerTag[i] = (uint8_t)request[15+queueNameSize+i];
        }
    }

    uint32_t length;
    /*class(2) + method(2) + consumerTagSize(1) + consumerTag*/
    length = 5 + consumerTagSize;
    length = htonl(length);

    uint8_t type[] = {0x01};
    uint8_t channel[] = {0x00, 0x01};
    uint8_t classs[] = {0x00, 0x3c};
    uint8_t method[] = {0x00, 0x15};
    uint8_t sizeC[] = {(uint8_t)consumerTagSize};
    uint8_t end[] = {0xce};

    write(connfd, type, 1);
    write(connfd, channel, 2);
    write(connfd, (uint8_t*)&length, 4);
    write(connfd, classs, 2);
    write(connfd, method, 2);
    write(connfd, sizeC, 1);
    write(connfd, consumerTag, (uint8_t)consumerTagSize);
    write(connfd, end, 1);

    ret[0] = queueName;
    ret[1] = consumerTag;

    return ret;
}

/*publica a mensagem na fila especificada pelo cliente*/
void Basic_Publish(queueList *globalList, int connfd, char* request){
    int i;

    ssize_t n;
    char aux[MAX_BUFFER_SIZE];
    uint8_t exchageNameSize, routingKeySize;
    uint64_t bodySize = 0;
    uint8_t *routingKey, *payload = NULL;    

    /*Nos exemplos de captura do Wireshark a exchange é uma string vazia
    * Default exchange: When you use default exchange, your message is delivered to the queue with a 
    * name equal to the routing key of the message. Every queue is automatically bound to the default 
    * exchange with a routing key which is the same as the queue name. 
    */
    exchageNameSize = charToInt(&(request[13]), 1);

    routingKeySize = charToInt(&(request[14+exchageNameSize]), 1);
    routingKey = (uint8_t*)malloc(sizeof(uint8_t)*routingKeySize);
    for(i=0; i<routingKeySize; i++){
        routingKey[i] = (uint8_t)request[15+exchageNameSize+i];
    }

    /*Lê a segunda frame que é enviada pelo cliente no publish - content header (2)*/
    n = read(connfd, aux, 7);
    if(n <= 0){
        close(connfd);
        return;
    }

    int length = charToInt(&aux[3], 4);
    n = read(connfd, aux+7, length+1);
    if(n <= 0){
        close(connfd);
        return;
    }

    bodySize = charToLongLong(&aux[11], 8);


    /*Lê a terceira frame - content body (3)*/
    n = read(connfd, aux, 7);
    if(n <= 0){
        close(connfd);
        return;
    }

    length = charToInt(&aux[3], 4);
    n = read(connfd, aux+7, length+1);
    if(n <= 0){
        close(connfd);
        return;
    }    

    if(bodySize != 0){
        payload = (uint8_t*)malloc(sizeof(uint8_t)*bodySize);
        for(uint64_t j=0; j<bodySize; j++){
            payload[j] = (uint8_t)aux[7+j];
        }
    }

    Basic_Deliver(globalList, routingKey, payload, bodySize);
}