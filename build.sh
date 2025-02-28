#!/bin/bash

set -ex

mkdir -p ./build

files=(./src/ds.c ./src/cam.c ./src/nav.c ./src/lexer.c ./src/draw.c ./src/explorer.c ./src/main.c)
dest="./build/ReD"
gcc "${files[@]}" -Wall -Wextra -o "${dest}" -L./raylib/lib -l:libraylib.a -lGL -lm
rm -f *.o

cp -r ./assets ./build/

if [ $# -eq 0 ]; then
	echo '...'
else
    ./build/ReD $1
fi
