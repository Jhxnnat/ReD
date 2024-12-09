#!/bin/bash

gcc ds.c cam.c nav.c ins.c lexer.c draw.c explorer.c main.c -o ReD -Wall -Wextra -l:libraylib.a -lGL -lm

rm -f *.o
