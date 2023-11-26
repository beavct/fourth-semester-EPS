PP=g++
COMPFLAGS= -Wall -pedantic -O2 -g
OTHERS= -lpthread

###################################################################################

JOGO = jogo.o partida.o labirinto.o personagem.o fantasma.o 
OBJS = server.o client.o estados.o

###################################################################################

SERVER= server.cpp
CLIENT= client.cpp
MAQ = estados.cpp
PACMAN = jogo.cpp partida.cpp labirinto.cpp personagem.cpp fantasma.cpp

###################################################################################

all: ep

partida.o: partida.cpp
labirinto.o: labirinto.cpp 
personagem.o: personagem.cpp
fantasma.o: fantasma.cpp
jogo.o: jogo.cpp

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
	@rm -rf *.o