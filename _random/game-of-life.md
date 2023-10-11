---
title: Conway's Game of Life
layout: post
tags: [javascript, html, css, github, rust, wasm]
date: 6 Oct 2023
comments: true
---

# About

I have built this project using WASM. I have wrote the logic of the game in
Rust, compiled it to WASM and then hosted the game using an HTML website.

The website is hosted on Github Pages.

<style>
  #wrap {
    width: 100%;
    height: 650px;
    overflow: hidden;
  }
  #scaled-frame {
    width: 2000px;
    height: 1000px;
    transform: scale(0.5);
    transform-origin: 0 0;
  }
</style>

<div id="wrap">
    <iframe
        id="scaled-frame"
        scrolling="no"
        title="Game Of Life"
        src="https://alexjercan.github.io/wasm-game-of-life/"
        frameborder="0"
    >
    </iframe>
</div>

The controls of the application are at the bottom of the screen. You can paint
different shapes, pause, play frame by frame, randomize and clear the screen.

# Usage

To start the application you will need to run

```console
npm run serve
```

# Conclusion

- [GitHub Repo](https://github.com/alexjercan/wasm-game-of-life)
