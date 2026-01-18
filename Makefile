
CFLAGS+=-O2 -I/usr/include/SDL -I/usr/include/freetype2 -I./
LDLIBS+=-lSDL -lm -lpthread -lSDL_ttf -lSDL_image -lfreetype -fopenmp

all: test

test: sdlmm.o 
babylon3D_cube: exams/babylon3D_cube.c sdlmm.c babylon3D.c
	gcc -O2 -I/usr/include/SDL -I/usr/include/freetype2 -I../ exams/babylon3D_cube.c ./sdlmm.c -lSDL -lm -lpthread -lSDL_ttf -lSDL_image -lfreetype -fopenmp -o babylon3D_cube
