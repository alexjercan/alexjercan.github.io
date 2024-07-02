---
title: Breakout Arduino
layout: post
tags: [c, arduino]
date: 2 Jul 2024
comments: true
---

# About

# Pong in C

This is just a small example using Raylib that shows what we expect to see on
the arduino. Not the best game ever, but it is just a demo for playing more wit
C and arduino.

## Quickstart

```console
mkdir build
cd build
cmake ..
cd ..
cmake --build ./build
./build/main
```

# Breakout in Arduino

Pong like game on Arduino.

## Quickstart

Assemble the parts as shown in the schematic and then connect the Arduino to
the USB. You will need some dependencies installed and to have Arduino on
/dev/ttyACM0, but other than that it should all be good to go.

<p align="center">
  <img src="/images/breakout-arduino/schematic.png" width="1000"/>
</p>

```console
make build
sudo make copy
```

# Conclusion

- [GitHub Repo](https://github.com/alexjercan/pong-breakout-arduino)

