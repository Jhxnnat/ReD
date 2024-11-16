#!/bin/bash
gcc -c -I. text.c
gcc -c -I. main.c
gcc -o red main.o text.o -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

rm -f *.o

# gcc ./main.c -o ./main -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
# ./main
