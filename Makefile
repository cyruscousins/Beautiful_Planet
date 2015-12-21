
DBGFLAGS=-O1 -g3 -Wall -Wno-unused-function -DDEBUG
OPTFLAGS=-Ofast -ffast-math -DNDEBUG -march=native -g#-m32 #Causes a compiler issue: probably 32 bit library related.

CFLAGS=$(DBGFLAGS) -std=c11
#CFLAGS=$(OPTFLAGS) -std=c11 -flto

LFLAGS=$(OPTFLAGS) -fuse-linker-plugin -flto -fwhole-program
LIB=-lm -lX11

UNAME := $(shell uname -s)
ifeq ($(UNAME),Darwin)
	LFLAGS += -L/usr/X11/lib
endif

all: test

wind.o: wind.h wind.c image.h
	gcc $(CFLAGS) wind.c -c

image.o: image.h image.c
	gcc $(CFLAGS) image.c -c

main.o: main.c image.h noise.h art.h
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

art.o: art.c art.h image.h
	gcc $(CFLAGS) art.c -c

x11.o: x11.c x11.h image.h
	gcc $(CFLAGS) x11.c -c

test: wind.o image.o main.o draw.o potentials.o noise.o parametric.o convolution.o filters.o art.o x11.o
	gcc $(LFLAGS) wind.o image.o main.o draw.o potentials.o noise.o parametric.o convolution.o filters.o art.o x11.o $(LFLAGS) $(LIB) -o test

clean:
	rm -f *.o *.gch
