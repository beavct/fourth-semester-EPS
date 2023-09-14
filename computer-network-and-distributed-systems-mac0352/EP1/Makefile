PP=gcc
COMPFLAGS= -Wall -ansi -pedantic -O2
OTHERS= 

###################################################################################

OBJS= 
MAIN= redes-servidor-exemplo-ep1.c

###################################################################################

all: ep

main.o: redes-servidor-exemplo-ep1.c

ep: ${OBJS}
		${PP} ${COMPFLAGS} -o ep1 ${MAIN} ${OTHERS}
		@rm -rf *.o 