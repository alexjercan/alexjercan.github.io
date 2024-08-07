---
title: BrainFuck Compiler
layout: post
tags: [c, data structures, algorithms]
date: 2 Jul 2024
comments: true
---

# About

BrainFuck to Assebly compiler. It uses nasm assembler to compile to machine
code.

I recommend trying out some of the examples from <http://brainfuck.org/>

See the `c` or `zig` directories to get started.

You can also run the checker using `./checker.sh <project>` where project can
be `c` or `zig`.

### Calculator Example

The calculator needs to have the input numbers with the same length. They can
be padded with zero.

```brainfuck
- marker
>,--------------------------------[>>>>>>>>,--------------------------------] read
- marker
< move on last digit +[-<+]- go back to marker 0 > move on first digit
+[- ---------------- >>>>>>>>+]- make all digits between 0 and 9
< move on last digit +[-<+]- go back to marker 0
>>,----------[>>>>>>>>,----------] read
- marker <
< move on last digit +[-<+]- go back to marker 0 >> move on first digit
+[- -------------------------------------- >>>>>>>>+]- make all digits between 0 and 9
< move on last marker
< move on last digit +[-<+]- go back to marker 0
> we are on the first number here

+[->+]- go last marker < move on last digit

+[-
    <<<<<<<

    # add two numbers
    >>>>>>>>>>>> [- <<<<<<<<<<<< + >>>>>>>>>>>>] <<<<<<<<<<<< add the carry
    [->+<]> > ++++++++++ > + << put 10 and 1 for divmod

    # Divmod Algorithm
    # n d 1 0 0 0
    [->-[>+>>]>[[-<+>]+>+>>]<<<<<]
    # 0 0 d_n%d n%d1 n/d 0 0

    # set modulo to actually n%d
    >>-

    # go to the last digit of the next number
    <<<<

    # check if we find a marker
    +
]-

# show the carry which will also be like zero in case no numbers
>>>>>
[++++++++++++++++++++++++++++++++++++++++++++++++ . [-]]

# go to start of the first number
<<<<
+[-
    >>> go to the quot

    ++++++++++++++++++++++++++++++++++++++++++++++++ .

    >>>>> go to the start of next number
    +
]-
```

# Conclusion


- [GitHub Repo](https://github.com/alexjercan/bf-compiler)
