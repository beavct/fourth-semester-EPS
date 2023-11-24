PP=g++
COMPFLAGS= -Wall -pedantic -O2
OTHERS= -lpthread

###################################################################################

JOGO = partida.o labirinto.o personagem.o fantasma.o
OBJS = server.o client.o eestados.o

###################################################################################

SERVER= server.cpp
CLIENT= client.cpp
MAQ = estados.cpp
PACMAN = partida.cpp

###################################################################################

all: ep

partida.o: partida.h
labirinto.o: labirinto.h 
personagem.o: personagem.h 
fantasma.o: fantasma.h

server.o: server.cpp
client.o: client.cpp
estados.o: estados.cpp

ep: ${OBJS}
	${PP} ${COMPFLAGS} -o server ${SERVER} ${OTHERS}
	${PP} ${COMPFLAGS} -o client ${CLIENT} ${OTHERS}
	${PP} ${COMPFLAGS} -o estados ${MAQ}
	@rm -rf *.o 

jogo: ${JOGO}
	${PP} ${COMPFLAGS} -o jogo ${PACMAN}