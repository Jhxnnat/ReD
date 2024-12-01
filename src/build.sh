#!/bin/bash

gcc ds.c cam.c nav.c ins.c lexer.c draw.c main.c -o red -Wall -Wextra -l:libraylib.a -lGL -lm

rm -f *.o
