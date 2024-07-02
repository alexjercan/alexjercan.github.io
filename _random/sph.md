---
title: Smoothed Particle Hydrodynamics in C
layout: post
tags: [c, data structures, algorithms]
date: 2 Jul 2024
comments: true
---

# About

My attempt at implementing
[SPH](https://en.wikipedia.org/wiki/Smoothed-particle_hydrodynamics) in C by
following this
[paper](https://web.archive.org/web/20160910114523id_/http://www.astro.lu.se:80/~david/teaching/SPH/notes/annurev.aa.30.090192.pdf).

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
        title="Particle Simulator"
        src="https://alexjercan.github.io/sph-c"
        frameborder="0"
    >
    </iframe>
</div>

For now it is pretty much a work in progress.

# Quickstart

```console
mkdir -p build
cd build
cmake ..
cd ..
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 ./build/ # for lsp
cmake --build ./build
./build/main
```

# Conclusion

- [GitHub Repo](https://github.com/alexjercan/sph-c)
