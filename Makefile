CC=g++
CFLAGS=-I.
DEPS = custom.h

all: main

main: main.cpp glad.c
	g++ -o main main.cpp glad.c -lGL -lglfw -lftgl -lSOIL -ldl -I/usr/local/include -I/usr/local/include/freetype2 -L/usr/local/lib

clean:
	rm main
