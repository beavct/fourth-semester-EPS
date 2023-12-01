PP=g++
COMPFLAGS= -Wall -pedantic -O2
OTHERS= -lpthread

###################################################################################

JOGO = jogo.o partida.o labirinto.o personagem.o fantasma.o 
OBJS = server.o client.o funcAux.o

###################################################################################

SERVER= server.cpp funcAux.cpp
CLIENT= client.cpp funcAux.cpp
PACMAN = jogo.cpp partida.cpp labirinto.cpp personagem.cpp fantasma.cpp

###################################################################################

all: rede jogo

partida.o: partida.cpp
labirinto.o: labirinto.cpp 
personagem.o: personagem.cpp
fantasma.o: fantasma.cpp
jogo.o: jogo.cpp

server.o: server.cpp
client.o: client.cpp
funcAux.o: funcAux.cpp

rede: ${OBJS}
	${PP} ${COMPFLAGS} -o server ${SERVER} ${OTHERS}
	${PP} ${COMPFLAGS} -o client ${CLIENT} ${OTHERS}
	@rm -rf *.o 

jogo: ${JOGO}
	${PP} ${COMPFLAGS} -o jogo ${PACMAN}
	@rm -rf *.o