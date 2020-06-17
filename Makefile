
CFLAGS+=-O2 -I/usr/include/SDL -I/usr/include/freetype2 -I./
LDLIBS+=-lSDL -lm -lpthread -lSDL_ttf -lSDL_image -lfreetype -fopenmp

all: test

test: sdlmm.o 
