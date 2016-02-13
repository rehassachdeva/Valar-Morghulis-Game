CC=g++
CFLAGS=-I.
DEPS = custom.h

all: main

main: main.cpp glad.c
	g++ -w -DLIBAuDIO -o main main.cpp glad.c -lalut -lGL -lglfw -lftgl -lopenal -lSOIL -ldl -I/usr/local/include -I/usr/local/include/freetype2 -L/usr/local/lib

clean:
	rm main
