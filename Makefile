
#Configuration (Supporting debug, release, and local builds). 

GENFLAGS=-std=gnu11
DBGFLAGS=$(GENFLAGS) -O1 -g3 -Wall -Wno-char-subscripts -Werror=implicit -Werror=incompatible-pointer-types -DDEBUG
OPTFLAGS=$(GENFLAGS) -Ofast -ffast-math -DNDEBUG -flto #-m32 #Causes a compiler issue: probably 32 bit library related.

#Default mode to debug.
ifeq ($(strip $(mode)),)
	mode=debug
endif

#Use the mode option to set flags.
#TODO I can't seem to get the else if syntax working, so this is a bit verbose.
ifeq ($(mode), debug)
	CFLAGS=$(DBGFLAGS)
	FOUNDMODE=true
endif
ifeq ($(mode), release)
	CFLAGS=$(OPTFLAGS)
	FOUNDMODE=true
endif
ifeq ($(mode), local)
	CFLAGS=$(OPTFLAGS) -march=native
	FOUNDMODE=true
endif
#ifndef ($(FOUNDMODE))
#	$(error Unrecognized mode option "$(mode)", please select "debug", "release", or "local".)
#endif

#Libraries and linkers.
LFLAGS = $(OPTFLAGS) -fuse-linker-plugin -flto -fwhole-program
LIB = -lm -lX11

#Due to Apple's policy of punishing anyone foolish enough to want to develop for their platform, the following compatibility option is required to link against the X11 window management library.
UNAME := $(shell uname -s)
ifeq ($(UNAME),Darwin)
	LFLAGS += -L/usr/X11/lib
endif

#Object files for each module in Beautiful Planet.
OBJ =  wind.o image.o draw.o potentials.o noise.o parametric.o convolution.o filters.o art.o x11.o global.o

all: bp libbp.a

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

global.o: global.c global.h
	gcc $(CFLAGS) global.c -c


#Executable
bp: $(OBJ) main.o
	gcc $(LFLAGS) $(OBJ) main.o $(LFLAGS) $(LIB) -o bp

#Library
libbp.a: $(OBJ)
	ar rcs libbp.a $(OBJ) -o libbp.a


clean:
	rm -f *.o *.gch *~ bp libbp.a

