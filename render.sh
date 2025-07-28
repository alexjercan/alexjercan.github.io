#!/bin/sh

set -e
rm -rf build
rm -rf dist
mkdir -p build
mkdir -p dist
touch ./build/post.h
touch ./build/rss.h

cp *.css ./dist
cp favicon.ico ./dist

run_main() {
    post_name=$(basename "$1")
    mkdir -p ./dist/"${post_name%.*}"

    clang -I./build -g main.c -o ./build/main
    ./build/main generate-post "$1"
    clang -I./build -g main.c -o ./build/main
    ./build/main render-post
}

for file in $(ls posts/*.md | sort); do
    run_main "$file"
done

new_index=$(find ./dist/* -name *.html -print | sort | tail -n 1)
cp "$new_index" ./dist/index.html

cp ./templates/404.html ./dist

clang -I./build -g main.c -o ./build/main
./build/main generate-rss $(ls -t posts/* | head -n 4)
clang -I./build -g main.c -o ./build/main
./build/main render-rss
