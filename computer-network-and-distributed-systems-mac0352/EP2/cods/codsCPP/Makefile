PP=g++
COMPFLAGS= -Wall -pedantic -O2
OTHERS= -lpthread

###################################################################################

#OBJS= extraFunctions.o
SERVER= server.cpp
CLIENT= client.cpp
MAQ = estados.cpp

###################################################################################

all: ep

#extraFunctions.o: extraFunctions.c

server.o: server.cpp
client.o: client.cpp
estados.o: estados.cpp

ep: ${OBJS}
		${PP} ${COMPFLAGS} -o server ${SERVER} ${OTHERS}
		${PP} ${COMPFLAGS} -o client ${CLIENT} ${OTHERS}
		${PP} ${COMPFLAGS} -o estados ${MAQ}
		@rm -rf *.o 