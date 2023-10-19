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
#define MAX_BUFFER_SIZE 4096
#define NUM_THREADS 150
#define htonll(x) ((((uint64_t)htonl(x)) << 32) + htonl((x) >> 32))

/*STRUCTS NOVAS*/
/* Estrutura de argumentos para as threads */
struct ThreadArgs {
    int connfd;
};

typedef struct socketNode{
    int connfd;
    uint8_t* consumerTag;
    struct socketNode* next;
    struct socketNode* ant;
    int head;
}socketNode;

typedef struct queueNode{
    uint8_t* queueName;
    struct queueNode* next;
    struct socketNode* socketHead;
    int socketSize;
    uint64_t deliveryTag;

}queueNode;  

typedef struct queue{
    struct queueNode* head;
    int queueSize;
}queueList;


/*VARIÁVEIS GLOBAIS*/
uint64_t deliveryTag = 1;
queueList *globalList;

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

    /*Chegamos no final da lista de filas e não encontramos a fila correta
    Então ela não foi declarada anteriormente e o socket é fechado*/
    //if(aux == NULL)
    //    return 0;

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

    //if(aux->socketSize == 1){
    //    auxS->next = auxS2;
    //    auxS->ant = auxS2;
//
    //    auxS2->next = auxS;
    //    auxS2->ant = auxS;
    //}
    //else{
    //    socketNode *auxAnt = auxS->ant;
    //    auxAnt->next = auxS2;
    //    auxS2->ant = auxAnt;
    //    auxS2->next = auxS;
    //    auxS->ant = auxS2;
    //}

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

    /*write(connfd, "0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x0a, 0x00, 0x0a", 64);
    write(connfd, "0x64, 0x61, 0x6e, 0x69, 0x65, 0x6c, 0x20, 0x6d, 0x65, 0x20, 0x64, 0x61, 0x20, 0x31, 0x30, 0x20, 0x70, 0x66, 0x76, 0x21", 118);*/

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
void Basic_Deliver(uint8_t *routingKey, uint8_t *payload, uint64_t bodySize){
    queueNode *aux = globalList->head;

    while(aux != NULL){
        if(strcmp((const char*)aux->queueName, (const char*)routingKey) == 0)
            break;

        aux = aux->next;
    }

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

    /*Envia o segundo frame*/
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


    /*Envia a terceira frame*/
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

void Basic_Ack(int connfd){
    char request[MAX_BUFFER_SIZE];
    ssize_t n;

    printf("entrou no ack");

    /*Basic.Ack por parte do cliente*/
    n = read(connfd, request, 7);
    if(n <= 0)
        close(connfd);

    printf("entrou no ack2");
    
    
    int length = charToInt(&request[3], 4);

    n = read(connfd, request+7, length + 1);
    if(n <= 0)
        close(connfd);

    printf("leu o ack");
}

/*Função do consumidor da mensagem*/
uint8_t** Basic_Consume(int connfd, char* request){

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

/*SLA SLA OQ - tem q ver se os campos q eu to olhando tão certos*/
void Basic_Publish(int connfd, char* request){
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

    /*Lê o Channel.Close que o cliente manda*/
    n = read(connfd, request, 7);
    if(n <= 0){
        close(connfd);
    }

    length = charToInt(&request[3], 4);
    n = read(connfd, request+7, length + 1);

    Basic_Deliver(routingKey, payload, bodySize);
}

/*Função paralelizada*/
void *makeConnection(void *arg){
    /*Necessário por conta das threads*/
    struct ThreadArgs *args = (struct ThreadArgs *)arg;
    int connfd = args->connfd;

    /*Leitura do pacote e identificação do método*/
    char request[MAX_BUFFER_SIZE];
    int methodID;
    ssize_t n;
    int consumer = 0;
    printf("[Uma conexão aberta]\n");

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
                uint8_t **ret;

                consumer = 1;

                ret = Basic_Consume(connfd, request);

                //int success = findQueue(globalList, (uint8_t*)request);
                //if(!success){
                //    printf("Essa fila não existe :(\n");
                //    exit(7);
                //}
                
                reallocSocket(globalList, connfd, ret[0], ret[1]);

            }
            /*Publicar mensagem*/
            else if(methodID == 40){
                Basic_Publish(connfd, request);
                Basic_Ack(connfd);
                Channel_Close(connfd);
                Connection_Close(connfd);
            }


    }


    /*Se for um consumidor ele fica esperando chegar mensagens*/
    if(!consumer){
            /*Fecha a conexão do socket*/
            close(connfd);
            printf("[Uma conexão fechada]\n");
    }

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
