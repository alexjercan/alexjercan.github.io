#!/bin/sh

set -e
rm -rf build
rm -rf dist
mkdir -p build
mkdir -p dist
touch ./build/template.h
touch ./build/rss.h

cp *.css ./dist

run_main() {
    clang -I./build -g main.c -o ./build/main
    ./build/main generate "$1"
    clang -I./build -g main.c -o ./build/main
    ./build/main render
}

for file in $(ls posts/*.md | sort); do
    run_main "$file"
done

new_index=$(find ./dist -name *.html -print | sort | tail -n 1)
cp "$new_index" ./dist/index.html

clang -I./build -g main.c -o ./build/main
./build/main generate-rss $(ls -t posts/* | head -n 4)
clang -I./build -g main.c -o ./build/main
./build/main render-rss
