// Daniel
// g++ -Wall -pedantic -O2 -o clienteTesteUDP clienteTesteUDP.cpp | rm *.o

/* Código simples de um cliente de echo usando UDP.
 * Não é o código ideal mas é suficiente para exemplificar os
 * conceitos da disciplina de redes de computadores.
 *
 * Prof. Daniel Batista em 17/05/2021
 *
 * Bugs? Tente consertar primeiro! Depois me envie email :) batista@ime.usp.br
 */

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>

#define MAXLINE 4096

int main(int argc, char **argv) {
    int	sockfd, n;
    char	recvline[MAXLINE + 1];
    struct  sockaddr_in servaddr;
    struct  sockaddr_in dadosLocal;
    int     dadosLocalLen,servaddrLen;
    struct  hostent *hptr;
    char    enderecoIPServidor[INET_ADDRSTRLEN];

	if (argc != 3) {
        fprintf(stderr,"Uso: %s <Endereco IP|Nome> <Porta>\n",argv[0]);
		exit(1);
	}

    if ( (hptr = gethostbyname(argv[1])) == NULL) {
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
	/* if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) { */
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
	    perror("socket error");
        exit(1);
    }

    bzero(&servaddr, sizeof(servaddr));
    dadosLocalLen=sizeof(dadosLocal);
    bzero(&dadosLocal, dadosLocalLen);
    servaddr.sin_family = AF_INET;
    servaddr.sin_port   = htons(atoi(argv[2]));

    /* UDP: É necessário passar o tamanho da estrutura para usar na função que
     * envia mensagem */
    servaddrLen=sizeof(servaddr); 

    if (inet_pton(AF_INET, enderecoIPServidor, &servaddr.sin_addr) <= 0) {
        perror("inet_pton error");
        exit(1);
    }

    /* UDP: Não há conexão com o protocolo UDP */
	/* if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
		perror("connect error");
		exit(1);
	}
   */
   
    if (getsockname(sockfd, (struct sockaddr *) &dadosLocal, (socklen_t *) &dadosLocalLen)) {
        perror("getsockname error");
        exit(1);
    }

    n=1;
    while (n != 0)  {
        if ((fgets(recvline,MAXLINE,stdin)) != NULL) {
            if (!strcmp(recvline,"exit\n"))
                n=0;
            else {
                /* UDP: Agora cada datagrama é independente pois não há
                 * conexão, então a cada envio de mensagem é necessário
                 * informar para onde ela vai */
                /* write(sockfd, recvline, strlen(recvline)); */
                sendto(sockfd,recvline,strlen(recvline),0,(struct sockaddr *)&servaddr,(socklen_t)servaddrLen);
                /* UDP: O recebimento de mensagem também tem que ser feito
                 * independente, pois não há mais conexão */
                /* n=read(sockfd,recvline,MAXLINE); */
                n=recvfrom(sockfd,recvline,MAXLINE,0,(struct sockaddr *)&servaddr,(socklen_t *)&servaddrLen);
                recvline[n]=0;
                if ((fputs(recvline,stdout)) == EOF) {
                   perror("fputs error");
                   exit (1);
                }
         }
      }
      else
        n=0;
    }

	if (n < 0) {
	    perror("read error");
	    exit(1);
	}
   
	exit(0);
}