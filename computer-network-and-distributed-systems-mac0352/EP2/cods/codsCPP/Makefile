PP=g++
COMPFLAGS= -Wall -pedantic -O2
OTHERS= -lpthread

###################################################################################

#OBJS= extraFunctions.o
SERVER= server.cpp
<<<<<<< HEAD
CLIENT= client.cpp
MAQ = estados.cpp
=======
>>>>>>> 48c989c651ad8e7af41ea3bd589485918557ede4

###################################################################################

all: ep

#extraFunctions.o: extraFunctions.c

server.o: server.cpp
client.o: client.cpp
<<<<<<< HEAD
estados.o: estados.cpp

ep: ${OBJS}
		${PP} ${COMPFLAGS} -o server ${SERVER} ${OTHERS}
		${PP} ${COMPFLAGS} -o client ${CLIENT} ${OTHERS}
		${PP} ${COMPFLAGS} -o estados ${MAQ}
=======

ep: ${OBJS}
		${PP} ${COMPFLAGS} -o server ${SERVER} ${OTHERS}
		${PP} ${COMPFLAGS} -o client client.cpp
>>>>>>> 48c989c651ad8e7af41ea3bd589485918557ede4
		@rm -rf *.o 