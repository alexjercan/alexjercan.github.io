---
title: Walker AI
layout: post
tags: [python, linux, reinforcement learning]
date: 11 Oct 2023
comments: true
---

# About

Implement NEAT algorithm to solve the BipedalWalker environment from OpenAI
gym. It uses the [neat-python](https://github.com/CodeReclaimers/neat-python)
for the genetic evolution algorithm.

<div class="video-container" align="center">
    <iframe
        width="840"
        height="478"
        src="https://www.youtube.com/embed/YluG5tCYEdY"
        title="YouTube video player"
        frameborder="0"
        allow="accelerometer; autoplay; clipboard-write; encrypted-media; gyroscope; picture-in-picture; web-share"
        allowfullscreen>
    </iframe>
</div>

# Usage

To evolve an agent you have to install the dependencies and then run the evolve
script.

```console
make install
make evolve
```

# Conclusion

- [GitHub Repo](https://github.com/alexjercan/walker-ai)
