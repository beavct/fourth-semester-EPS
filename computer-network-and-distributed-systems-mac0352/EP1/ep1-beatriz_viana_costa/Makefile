PP=gcc
COMPFLAGS= -Wall -pedantic -O2
OTHERS= extraFunctions.c -lpthread

###################################################################################

OBJS= extraFunctions.o
MAIN= main.c

###################################################################################

all: ep

extraFunctions.o: extraFunctions.c

main.o: redes-servidor-exemplo-ep1.c

ep: ${OBJS}
		${PP} ${COMPFLAGS} -o ep1 ${MAIN} ${OTHERS}
		@rm -rf *.o 