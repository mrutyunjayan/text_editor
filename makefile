INCLUDE = ./include/ 
CC = clang
CFLAGS = `sdl2-config --cflags --libs` -I$(INCLUDE) code/sdl2_main.c -Wall -Wno-unused-parameter -Wno-undef -Wno-unused-macros -Wno-extra-semi-stmt -Wno-unused-function -g  

build: code/*.c
	$(CC) $(CFLAGS) -o build/jed 

run:
	./build/jed

clean:
	rm ./build/jed

.PHONY: build run clean
