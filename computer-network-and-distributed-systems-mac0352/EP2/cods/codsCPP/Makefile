PP=g++
COMPFLAGS= -Wall -pedantic -O2
OTHERS= -lpthread

###################################################################################

#OBJS= extraFunctions.o
SERVER= server.cpp

###################################################################################

all: ep

#extraFunctions.o: extraFunctions.c

server.o: server.cpp
client.o: client.cpp

ep: ${OBJS}
		${PP} ${COMPFLAGS} -o server ${SERVER} ${OTHERS}
		${PP} ${COMPFLAGS} -o client client.cpp
		@rm -rf *.o 