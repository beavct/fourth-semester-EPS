PP=g++
COMPFLAGS= -Wall -pedantic -O2
OTHERS= -lpthread

###################################################################################

#OBJS= extraFunctions.o
MAIN= server.cpp

###################################################################################

all: ep

#extraFunctions.o: extraFunctions.c

main.o: server.cpp

ep: ${OBJS}
		${PP} ${COMPFLAGS} -o ep2 ${MAIN} ${OTHERS}
		@rm -rf *.o 