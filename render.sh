#!/bin/sh

set -e
mkdir -p build
mkdir -p dist
touch ./build/template.h

cp *.css ./dist
cp index.html ./dist

run_main() {
    clang -I./build -g main.c -o ./build/main
    ./build/main generate "$1"
    clang -I./build -g main.c -o ./build/main
    ./build/main render
}

for file in posts/*.md; do
    run_main "$file"
done
