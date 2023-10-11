---
title: Reddit Image Downloader
layout: post
tags: [cs, parallel, mpsc, windows]
date: 11 Oct 2023
comments: true
---

# About

C# project to help download images from <www.reddit.com>

This executable will download any amount of images on the provided subreddit
and save them in the current working directory. The images will be saved with
the name of the posts, removing any invalid character from the path.

# Usage

`path/to/executable/RIM_CLI.exe subreddit_name images_count [nr_threads]`

<div align="center">
  <img src="/images/rim/example.jpg" width="1000"/>
  <div align="center">Downloading some memes.</div>
  <br/>
</div>

<div align="center">
  <img src="/images/rim/compare.jpg" width="1000"/>
  <div align="center">Speedup comparison 8 threads vs 1 thread.</div>
  <br/>
</div>

# Conclusion

- [GitHub Repo](https://github.com/alexjercan/reddit-image-downloader)
