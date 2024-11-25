#!/bin/bash

RAYHEAD=../raylib/include/raylib.h
RAYLIB=../raylib/lib/libraylib.a
RAYDIR=../raylib

gcc -c ds.c
gcc -c cam.c
gcc -c nav.c 
gcc -c ins.c
gcc -c main.c

gcc -o red ds.o cam.o nav.o ins.o main.o -L../raylib/lib/libraylib.a -lraylib -lGL -lm

rm -f *.o
