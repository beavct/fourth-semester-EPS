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

#define LISTENQ 1
#define MAXDATASIZE 100
#define MAXLINE 4096


/*DEFINES NOVOS*/
#define MAX_FRAME_SIZE 131072


/*FUNÇÕES NOVAS*/

/*Se retornar 1 - Pode prosseguir para o Conection.Start, c.c. a conexão é recusada e o programa retorna a header correta*/
int readHeader(int connfd){
    /*A header que vamos ler tem 8 bytes*/
    char header[8];
    ssize_t n;

    n = read(connfd, header, 8);

    /*Envia a header de volta para o cliente*/

    /*Verifica se o cliente está pedindo uma conexão com a versão AMQP 0.9.1*/
    if(n <= 0 || strcmp("\x41\x4d\x51\x50\x00\x00\x09\x01", header) != 0){
        /*Retorna a header correta e fecha a conexão com o socket*/
        write(connfd, "\x41\x4d\x51\x50\x00\x00\x09\x01", 8);
        close(connfd);
        /*exit(0);*/
        /*return 0;*/
    }

    return 1;
}

/*Requeisição de Conexão*/
void Connection_Start(int connfd){
    char request[584];
    ssize_t n;


    /*Connection.Start pelo cliente*/
    n = read(connfd, request, 584);

    /*Chegou no fim do arquivo e não tem mais o que ser lido*/
    /*if(n == 0)
        printf("oie\n");*/


    /*Houve mensagem de Connection.Start*/
    /*HEX retirado do Wireshark, Connection.Start-OK pelo servidor*/
    write(connfd, "\x01\x00\x00\x00\x00\x01\x40\x00\x0a\x00\x0b\x00\x00\x01\x1c\x07" \
            "\x70\x72\x6f\x64\x75\x63\x74\x53\x00\x00\x00\x0a\x72\x61\x62\x62" \
            "\x69\x74\x6d\x71\x2d\x63\x07\x76\x65\x72\x73\x69\x6f\x6e\x53\x00" \
            "\x00\x00\x06\x30\x2e\x31\x31\x2e\x30\x08\x70\x6c\x61\x74\x66\x6f" \
            "\x72\x6d\x53\x00\x00\x00\x05\x4c\x69\x6e\x75\x78\x09\x63\x6f\x70" \
            "\x79\x72\x69\x67\x68\x74\x53\x00\x00\x00\x49\x43\x6f\x70\x79\x72" \
            "\x69\x67\x68\x74\x20\x28\x63\x29\x20\x32\x30\x30\x37\x2d\x32\x30" \
            "\x31\x34\x20\x56\x4d\x57\x61\x72\x65\x20\x49\x6e\x63\x2c\x20\x54" \
            "\x6f\x6e\x79\x20\x47\x61\x72\x6e\x6f\x63\x6b\x2d\x4a\x6f\x6e\x65" \
            "\x73\x2c\x20\x61\x6e\x64\x20\x41\x6c\x61\x6e\x20\x41\x6e\x74\x6f" \
            "\x6e\x75\x6b\x2e\x0b\x69\x6e\x66\x6f\x72\x6d\x61\x74\x69\x6f\x6e" \
            "\x53\x00\x00\x00\x28\x53\x65\x65\x20\x68\x74\x74\x70\x73\x3a\x2f" \
            "\x2f\x67\x69\x74\x68\x75\x62\x2e\x63\x6f\x6d\x2f\x61\x6c\x61\x6e" \
            "\x78\x7a\x2f\x72\x61\x62\x62\x69\x74\x6d\x71\x2d\x63\x0c\x63\x61" \
            "\x70\x61\x62\x69\x6c\x69\x74\x69\x65\x73\x46\x00\x00\x00\x3c\x1c" \
            "\x61\x75\x74\x68\x65\x6e\x74\x69\x63\x61\x74\x69\x6f\x6e\x5f\x66" \
            "\x61\x69\x6c\x75\x72\x65\x5f\x63\x6c\x6f\x73\x65\x74\x01\x1a\x65" \
            "\x78\x63\x68\x61\x6e\x67\x65\x5f\x65\x78\x63\x68\x61\x6e\x67\x65" \
            "\x5f\x62\x69\x6e\x64\x69\x6e\x67\x73\x74\x01\x05\x50\x4c\x41\x49" \
            "\x4e\x00\x00\x00\x0c\x00\x67\x75\x65\x73\x74\x00\x67\x75\x65\x73" \
            "\x74\x05\x65\x6e\x5f\x55\x53\xce"
            ,329);

}

/*Descrição da documentação: propose/negotiate connection tuning parameters*/
void Connection_Tune(int connfd){
    char tune[64];
    ssize_t n;

    /*recebe Connection.Tune do cliente*/
    n = read(connfd, tune, 64);

    /*Responde Connection.Tune-OK*/
    /*Não tenho certeza*/
    write(connfd, "\x01\x00\x00\x00\x00\x01\x40\x00\x0a\x00\x14\x00\x00\x00\x00\x0a\x14\x00", 19);

}

void Channel_Open(int connfd);

void Queue_Declare();

void Connection_Open();

void Channel_Close();

void Basic_Consume();


int main (int argc, char **argv) {
    /* Os sockets. Um que será o socket que vai escutar pelas conexões
     * e o outro que vai ser o socket específico de cada conexão
     */
    int listenfd, connfd;
    /* Informações sobre o socket (endereço e porta) ficam nesta struct
     */
    struct sockaddr_in servaddr;
    /* Retorno da função fork para saber quem é o processo filho e
     * quem é o processo pai
     */
    pid_t childpid;
    /* Armazena linhas recebidas do cliente
     */
    char recvline[MAXLINE + 1];
    /* Armazena o tamanho da string lida do cliente
     */
    ssize_t n;
   
    /*VARIÁVEIS NOVAS*/


    if (argc != 2) {
        fprintf(stderr,"Uso: %s <Porta>\n",argv[0]);
        fprintf(stderr,"Vai rodar um servidor de echo na porta <Porta> TCP\n");
        exit(1);
    }

    /* Criação de um socket. É como se fosse um descritor de arquivo.
     * É possível fazer operações como read, write e close. Neste caso o
     * socket criado é um socket IPv4 (por causa do AF_INET), que vai
     * usar TCP (por causa do SOCK_STREAM), já que o AMQP funciona sobre
     * TCP, e será usado para uma aplicação convencional sobre a Internet
     * (por causa do número 0)
     */
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket :(\n");
        exit(2);
    }

    /* Agora é necessário informar os endereços associados a este
     * socket. É necessário informar o endereço / interface e a porta,
     * pois mais adiante o socket ficará esperando conexões nesta porta
     * e neste(s) endereços. Para isso é necessário preencher a struct
     * servaddr. É necessário colocar lá o tipo de socket (No nosso
     * caso AF_INET porque é IPv4), em qual endereço / interface serão
     * esperadas conexões (Neste caso em qualquer uma -- INADDR_ANY) e
     * qual a porta. Neste caso será a porta que foi passada como
     * argumento no shell (atoi(argv[1])). No caso do servidor AMQP,
     * utilize a porta padrão do protocolo que você deve descobrir
     * lendo a especificaçao dele ou capturando os pacotes de conexões
     * ao RabbitMQ com o Wireshark
     */
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port        = htons(atoi(argv[1]));
    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
        perror("bind :(\n");
        exit(3);
    }

    /* Como este código é o código de um servidor, o socket será um
     * socket que escutará por conexões. Para isto é necessário chamar
     * a função listen que define que este é um socket de servidor que
     * ficará esperando por conexões nos endereços definidos na função
     * bind.
     */
    if (listen(listenfd, LISTENQ) == -1) {
        perror("listen :(\n");
        exit(4);
    }

    printf("[Servidor no ar. Aguardando conexões na porta %s]\n",argv[1]);
    printf("[Para finalizar, pressione CTRL+c ou rode um kill ou killall]\n");
   
    /* O servidor no final das contas é um loop infinito de espera por
     * conexões e processamento de cada uma individualmente
     */
	for (;;) {
        /* O socket inicial que foi criado é o socket que vai aguardar
         * pela conexão na porta especificada. Mas pode ser que existam
         * diversos clientes conectando no servidor. Por isso deve-se
         * utilizar a função accept. Esta função vai retirar uma conexão
         * da fila de conexões que foram aceitas no socket listenfd e
         * vai criar um socket específico para esta conexão. O descritor
         * deste novo socket é o retorno da função accept.
         */
        if ((connfd = accept(listenfd, (struct sockaddr *) NULL, NULL)) == -1 ) {
            perror("accept :(\n");
            exit(5);
        }
      
        /* Agora o servidor precisa tratar este cliente de forma
         * separada. Para isto é criado um processo filho usando a
         * função fork. O processo vai ser uma cópia deste. Depois da
         * função fork, os dois processos (pai e filho) estarão no mesmo
         * ponto do código, mas cada um terá um PID diferente. Assim é
         * possível diferenciar o que cada processo terá que fazer. O
         * filho tem que processar a requisição do cliente. O pai tem
         * que voltar no loop para continuar aceitando novas conexões.
         * Se o retorno da função fork for zero, é porque está no
         * processo filho.
         */
        if ( (childpid = fork()) == 0) {
            /**** PROCESSO FILHO ****/
            printf("[Uma conexão aberta]\n");
            /* Já que está no processo filho, não precisa mais do socket
             * listenfd. Só o processo pai precisa deste socket.
             */
            close(listenfd);
         
            /* Agora pode ler do socket e escrever no socket. Isto tem
             * que ser feito em sincronia com o cliente. Não faz sentido
             * ler sem ter o que ler. Ou seja, neste caso está sendo
             * considerado que o cliente vai enviar algo para o servidor.
             * O servidor vai processar o que tiver sido enviado e vai
             * enviar uma resposta para o cliente (Que precisará estar
             * esperando por esta resposta) 
             */

            /* ========================================================= */
            /* ========================================================= */
            /*                         EP1 INÍCIO                        */
            /* ========================================================= */
            /* ========================================================= */
            /* TODO: É esta parte do código que terá que ser modificada
             * para que este servidor consiga interpretar comandos AMQP
             */


            /*if(readHeader(connfd)){
                    Connection_Start(connfd);
                    Connection_Tune(connfd);
            }*/
            

            if(readHeader(connfd)){
                    Connection_Start(connfd);
                    Connection_Tune(connfd);

                    while ((n=read(connfd, recvline, MAXLINE)) > 0) {
                        recvline[n]=0;
                        printf("[Cliente conectado no processo filho %d enviou:] ",getpid());
                        if ((fputs(recvline,stdout)) == EOF) {
                            perror("fputs :( \n");
                            exit(7);
                        }

                        write(connfd, recvline, strlen(recvline));

                }
            }

            /* ========================================================= */
            /* ========================================================= */
            /*                         EP1 FIM                           */
            /* ========================================================= */
            /* ========================================================= */

            /* Após ter feito toda a troca de informação com o cliente,
             * pode finalizar o processo filho
             */
            printf("[Uma conexão fechada]\n");
            exit(0);
        }
        else
            /**** PROCESSO PAI ****/
            /* Se for o pai, a única coisa a ser feita é fechar o socket
             * connfd (ele é o socket do cliente específico que será tratado
             * pelo processo filho)
             */
            close(connfd);
    }
    exit(0);
}
