---
title: Fried YouTube
layout: post
tags: [bash, ffmpeg]
date: 9 Aug 2023
comments: true
---

# About

This is a really simple bash scripting project I did as a meme. The tool works
by downloading a video from youtube using
[youtube-dl](https://github.com/ytdl-org/youtube-dl), then it merges the video
with another video vertically. The same way people do it on TikTok to get more
views.

<div class="video-container" align="center">
	<iframe
        title="YouTube video player"
        width="840"
        height="478"
        src="https://www.youtube.com/embed/fDFg28phMs0"
        frameborder="0"
        allow="accelerometer; autoplay; clipboard-write; encrypted-media; gyroscope; picture-in-picture; web-share"
        allowfullscreen
    >
    </iframe>
</div>

It is possible to use any youtube video URL for both the top video and bottom
video. The only limitation is that you have to delete the `minecraft.mp4` when
you want to change the bottom video. I used the naming for cache, and because
you will probably have one video for multiple shorts.

# Quickstart

Run the script with the url that you want to watch. This will create a mp4 file
that you can watch.

```console
./fried.sh <URL/path> [optional_URL/path]
```

If you run it for the first time it will also have to download the minecraft
video, but it is what it is.

### FAQ

Q: It takes so long.\
A: I know, currently it does not support streaming so you have to wait for the
entire video to generate first. Trust me, the dopamine will be worth it.

Q: What to do while I wait for the video to render?\
A: Watch some TikTok.

Q: Can it also work on local files?\
A: Yes. If the url is a file on the local machine it will use that instead.

Q: Can I change the default minecraft video?\
A: Yes. Use the second argument as the url/path to the vertical bottom video.

# Conclusion

This is a toy project I used to learn more ffmpeg. You can use it if you want
to create a short video fast that uses some minecraft or subway surfers
background.

TLDR: Transform YouTube in TikTok from the CLI.

- [GitHub Repo](https://github.com/alexjercan/fried-youtube-cli)
