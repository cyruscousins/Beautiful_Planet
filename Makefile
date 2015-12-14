
DBGFLAGS=-O1 -g3 -Wall -Wno-unused-function -DDEBUG
OPTFLAGS=-Ofast -ffast-math -DNDEBUG -march=native -g#-m32 #Causes a compiler issue: probably 32 bit library related.

#CFLAGS=$(DBGFLAGS)
CFLAGS=$(OPTFLAGS) -flto

LFLAGS=$(OPTFLAGS) -lm
LIB=-lm

all: test

wind.o: wind.h wind.c image.h
	gcc $(CFLAGS) wind.c -c

image.o: image.h image.c
	gcc $(CFLAGS) image.c -c

main.o: main.c
	gcc $(CFLAGS) main.c -c

draw.o: draw.c draw.h image.h
	gcc $(CFLAGS) draw.c -c

potentials.o: potentials.c potentials.h
	gcc $(CFLAGS) potentials.c -c

noise.o: noise.c noise.h convolution.h
	gcc $(CFLAGS) noise.c -c

parametric.o: parametric.c parametric.h image.h
	gcc $(CFLAGS) parametric.c -c

convolution.o: convolution.c convolution.h
	gcc $(CFLAGS) convolution.c -c

filters.o: filters.c filters.h image.h
	gcc $(CFLAGS) filters.c -c

test: wind.o image.o main.o draw.o potentials.o noise.o parametric.o convolution.o filters.o
	gcc $(LFLAGS) wind.o image.o main.o draw.o potentials.o noise.o parametric.o convolution.o filters.o $(LIB) -o test

clean:
	rm *.o *.gch
