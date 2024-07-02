---
title: AStar
layout: post
tags: [c, wasm, javascript, data structures, algorithms]
date: 2 Jul 2024
comments: true
---

# About

Simple astar algorithm implemented in C and used in a WASM project.

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
        title="AStar"
        src="https://alexjercan.github.io/astar-wasm/"
        frameborder="0"
    >
    </iframe>
</div>

# Quickstart

Use mouse to toggle cells in the grid and then hit the `Run` button to create
the path from top-left to bottom-right.

```console
npm install
npm run serve
# open the browser at localhost:8080
```

# Conclusion

- [GitHub Repo](https://github.com/alexjercan/astar-wasm)
