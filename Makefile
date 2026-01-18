
# Compiler flags with SIMD optimizations
# SSE2 is enabled by default on x86_64, but we make it explicit
# Use -mavx for AVX support, -mavx2 for AVX2, -march=native for all CPU features
CFLAGS+=-O2 -I/usr/include/SDL -I/usr/include/freetype2 -I./ -msse2
LDLIBS+=-lSDL -lm -lpthread -lSDL_ttf -lSDL_image -lfreetype -fopenmp

# Uncomment for AVX support (requires CPU with AVX)
# CFLAGS+=-mavx

# Uncomment to enable all native CPU optimizations (recommended for best performance)
# CFLAGS+=-march=native

all: test

test: sdlmm.o 
babylon3D_cube: exams/babylon3D_cube.c sdlmm.c babylon3D.c
	gcc -O2 -I/usr/include/SDL -I/usr/include/freetype2 -I../ -msse2 exams/babylon3D_cube.c ./sdlmm.c -lSDL -lm -lpthread -lSDL_ttf -lSDL_image -lfreetype -fopenmp -o babylon3D_cube
