#!/bin/bash
# gcc -c -I. fs.c
gcc -g -c -I. ds.c
gcc -g -c -I. cam.c
gcc -g -c -I. nav.c 
gcc -g -c -I. ins.c
gcc -g -c -I. main.c
gcc -g -Wall -o red main.o ins.o nav.o cam.o ds.o -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

rm -f *.o

# gcc ./main.c -o ./main -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
# ./main
