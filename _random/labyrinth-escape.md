---
title: Escape the Labyrinth
layout: post
tags: [learn, practice, programming, javascript, game]
date: 25 Jul 2023
comments: true
---

# About

For this project I have used ChatGPT to generate an idea for a game. I have
used the following prompt `Give me a title for a game that I would have to
implement as a challenge` to get the title idea. Then I used the title to come
up with mechanichs for the game.

You have to use `wasd` to move around. The objective is to find the exit of the
maze. You have to avoid the guards and not step on traps when they are active.
The exit of the maze will be blocked (closed door) until you find all the keys.
You pick up keys by walking over them.

<style>
  #wrap {
    width: 100%;
    height: 650px;
    overflow: hidden;
  }
  #scaled-frame {
    width: 1408px;
    height: 896px;
    transform: scale(0.70);
    transform-origin: 0 0;
  }
</style>

<div id="wrap">
    <iframe
        id="scaled-frame"
        scrolling="no"
        title="Escape The Labyrinth"
        src="https://alexjercan.github.io/labyrinth-escape/"
        frameborder="0"
    >
    </iframe>
</div>

The game is written in plain javascript and can be hosted statically with

```console
python -m http.server
```

<div class="video-container" align="center">
	<iframe
        title="YouTube video player"
        width="840"
        height="478"
        src="https://www.youtube.com/embed/3pNTYmIrMwY"
        frameborder="0"
        allow="accelerometer; autoplay; clipboard-write; encrypted-media; gyroscope; picture-in-picture; web-share"
        allowfullscreen
    >
    </iframe>
</div>

- [Demo Game](https://alexjercan.github.io/labyrinth-escape/)
- [GitHub Repository](https://github.com/alexjercan/labyrinth-escape)
