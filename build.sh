#!/bin/bash

set -ex

files=(./src/ds.c ./src/cam.c ./src/nav.c ./src/ins.c ./src/lexer.c ./src/draw.c ./src/explorer.c ./src/main.c)
dest="./build/ReD"
gcc "${files[@]}" -o "${dest}" -Wall -Wextra -l:libraylib.a -lGL -lm
rm -f *.o

cp -r ./assets ./build/

