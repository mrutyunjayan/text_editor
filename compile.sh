#!/bin/zsh

clang `sdl2-config --cflags --libs` src/main.c -g -o build/jed 
