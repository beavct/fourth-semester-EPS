PP=gcc
COMPFLAGS= -Wall -pedantic -O2 -g
OTHERS= -lpthread

###################################################################################

OBJS= 
MAIN= redes-servidor-exemplo-ep1.c

###################################################################################

all: ep

main.o: redes-servidor-exemplo-ep1.c

ep: ${OBJS}
		${PP} ${COMPFLAGS} -o ep1 ${MAIN} ${OTHERS}
		@rm -rf *.o 