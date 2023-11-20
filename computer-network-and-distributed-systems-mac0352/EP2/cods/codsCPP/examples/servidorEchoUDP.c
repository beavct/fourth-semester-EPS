// g++ -Wall -pedantic -O2 -o servidorTesteUDP servidorTesteUDP.cpp
/* Código simples de um servidor de echo usando UDP.
 * Não é o código ideal mas é suficiente para exemplificar os
 * conceitos da disciplina de redes de computadores.
 *
 * Prof. Daniel Batista em 17/05/2021
 *
 * Bugs? Tente consertar primeiro! Depois me envie email :) batista@ime.usp.br
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

#define LISTENQ 1
#define MAXDATASIZE 100
#define MAXLINE 4096

int main (int argc, char **argv) {
    int    listenfd, connfd;
    struct sockaddr_in servaddr;
    struct sockaddr_in dadosRemoto;
    int    dadosRemotoLen;
    char   enderecoRemoto[MAXDATASIZE + 1];
    ssize_t  n;
    pid_t  childpid;
    char	recvline[MAXLINE + 1];

    if (argc != 2) {
        fprintf(stderr,"Uso: %s <Porta>\n",argv[0]);
		exit(1);
	}

    /* UDP: Sockets UDP tem que ser especificados com SOCK_DGRAM */
    /* if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) { */
    if ((listenfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    bzero(&servaddr, sizeof(servaddr));
    dadosRemotoLen=sizeof(dadosRemoto);
    bzero(&dadosRemoto, dadosRemotoLen);
    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port        = htons(atoi(argv[1]));

    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
        perror("bind");
        exit(1);
    }

   /* UDP: Sockets UDP não precisam ficar escutando pois não haverá
    * necessidade de ter filas de conexões completas/incompletas já
    * que não existe o conceito de conexão */
	/* if (listen(listenfd, LISTENQ) == -1) {
		perror("listen");
		exit(1);
	}*/

	for ( ; ; ) {
      /* UDP: Não há necessidade de aceitar a conexão porque ela não
       * vai ser colocada em nenhuma fila (não há conexão */
		/* if ((connfd = accept(listenfd, (struct sockaddr *) &dadosRemoto, (socklen_t *) &dadosRemotoLen)) == -1 ) {
			perror("accept");
			exit(1);
		} */

      /* UDP: Agora cada datagrama é independente pois não há
       * conexão, então a cada recebimento de mensagem é necessário
       * informar para onde ela vai e gravar de onde ela veio */
        n=recvfrom(listenfd,recvline,MAXLINE,0,(struct sockaddr *)&dadosRemoto,(socklen_t *)&dadosRemotoLen);

      /* UDP: As informações do socket remoto só podem ser informadas
       * depois que um pacote chegar pois só nesse momento os dados do
       * socket remoto serão conhecidos */
        printf("Dados do socket remoto: (IP: %s, PORTA: %d enviou mensagem)\n",inet_ntop(AF_INET, &(dadosRemoto.sin_addr).s_addr,enderecoRemoto,sizeof(enderecoRemoto)), ntohs(dadosRemoto.sin_port));

        recvline[n]=0;
        if ((fputs(recvline,stdout)) == EOF) {
            perror("fputs error");
            exit (1);
        }
      /* UDP: O envio de mensagem também tem que ser feito
       * independente, pois não há mais conexão */
        sendto(listenfd,recvline,n,0,(struct sockaddr *)&dadosRemoto,(socklen_t)dadosRemotoLen);
        printf("Dados do socket remoto: (IP: %s, PORTA: %d recebeu um datagrama [mas eu não garanto!])\n",inet_ntop(AF_INET, &(dadosRemoto.sin_addr).s_addr,enderecoRemoto,sizeof(enderecoRemoto)), ntohs(dadosRemoto.sin_port));
      
      /* UDP: Servidores UDP são concorrentes por definição, já que
       * não há mais necessidade de ter conexões e um processo
       * responsável por cada conexão */
      // if ( (childpid = fork()) == 0) { /* Se for zero está no processo filho */
      //    close(listenfd);  /* Fecha o socket que está escutando (só precisa de 1) */
      //    
      //    while ((n=read(connfd, recvline, MAXLINE)) > 0) {
      //       recvline[n]=0;
      //       if ((fputs(recvline,stdout)) == EOF) {
      //          perror("fputs error");
      //          exit (1);
      //       }
      //       write(connfd, recvline, strlen(recvline));
      //    }
      //    printf("Dados do socket remoto: (IP: %s, PORTA: %d desconectou)\n",inet_ntop(AF_INET, &(dadosRemoto.sin_addr).s_addr,enderecoRemoto,sizeof(enderecoRemoto)), ntohs(dadosRemoto.sin_port));
      //    exit (0);         /* Termina o processo filho */
      // }
      // 
      // /* Se for o pai continua a execução aqui... */
		// close(connfd);
	}
	return(0);
}