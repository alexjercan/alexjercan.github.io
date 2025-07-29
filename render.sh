#!/bin/sh

set -e
rm -rf build
rm -rf dist
mkdir -p build
mkdir -p dist
touch ./build/distr.h

cp *.css ./dist
cp favicon.ico ./dist

clang -I./build -g main.c -o ./build/main
./build/main generate
clang -I./build -g main.c -o ./build/main
./build/main render

new_index=$(find ./dist/* -name *.html -print | sort | tail -n 1)
cp "$new_index" ./dist/index.html

cp ./templates/404.html ./dist
