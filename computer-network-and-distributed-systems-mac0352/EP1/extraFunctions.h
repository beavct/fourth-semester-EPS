#pragma once

#include <netdb.h>

/*STRUCTS NOVAS*/
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

/* Estrutura de argumentos para as threads */
struct ThreadArgs {
    int connfd;
    queueList *globalList;
};

void printaHex(uint8_t* msg, int size);
void printaListas(queueList *q);
int charToInt(char* msg, int size);
uint64_t charToLongLong(char* msg, int size);
void removeConsumer(queueList *globalList, int connfd, uint8_t* queueName);
queueList* inicializateQueue();
queueList* reallocQueue(queueList* q, uint8_t *queueName);
int findQueue(queueList *q, uint8_t *queueName);
void reallocSocket(queueList* q, int socket, uint8_t *queueName, uint8_t* consumerTag);
uint8_t* generateConsumerTag();
int readHeader(int connfd);
void Connection_Start(int connfd);
void Connection_Tune(int connfd);
void Connection_Open(int connfd);
void Channel_Open(int connfd);
void Channel_Close(int connfd);
void Connection_Close(int connfd);
uint8_t* Queue_Declare(int connfd, char* request);
void Basic_Deliver(queueList *globalList, uint8_t *routingKey, uint8_t *payload, uint64_t bodySize);
void Basic_Ack(int connfd);
uint8_t** Basic_Consume(queueList *globalList, int connfd, char* request);
void Basic_Publish(queueList *globalList, int connfd, char* request);