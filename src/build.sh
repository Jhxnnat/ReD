#!/bin/bash
gcc -c -I. ds.c
gcc -c -I. nav.c 
gcc -c -I. ins.c
gcc -c -I. main.c
gcc -g -Wall -o red main.o ins.o nav.o ds.o -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

rm -f *.o

# gcc ./main.c -o ./main -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
# ./main
