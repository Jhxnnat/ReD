#!/bin/bash

cd ../src

files=(ds.c cam.c nav.c ins.c lexer.c draw.c explorer.c main.c)
dest="../build/ReD"
gcc "${files[@]}" -o "${dest}" -Wall -Wextra -l:libraylib.a -lGL -lm
rm -f *.o

cd ../build/

