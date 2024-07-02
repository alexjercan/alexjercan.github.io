---
title: Compiler in C Tutorial
layout: post
tags: [c, compiler, learn, practice]
date: 2 July 2024
comments: true
---

# About

A really simple programming language meant to be used as a tutorial for
compilers (really easy ones).

<div class="video-container" align="center">
	<iframe
        title="YouTube video player"
        width="840"
        height="478"
        src="https://www.youtube.com/embed/HOe2YFnzO2I"
        frameborder="0"
        allow="accelerometer; autoplay; clipboard-write; encrypted-media; gyroscope; picture-in-picture; web-share"
        allowfullscreen
    >
    </iframe>
</div>

```text
term = <input> | variable | literal
expression = term | term + term | ...
rel = term < term | ...
instr = variable = expression | <if> rel <then> instr | <goto> :label | <output> term | :label
```

Example of a program in this language

```text
n = input
i = 0
:label
output i
i = i + 1
if i < n then goto :label
if i < 10 then goto :label2
output 69
:label2
```

# Quickstart

```console
gcc -g main.c -o main
cat example.txt | ./main > example.asm
fasm example.asm
./example
```

# Conclusion

- [GitHub Repo](https://github.com/alexjercan/compiler-tutorial)
