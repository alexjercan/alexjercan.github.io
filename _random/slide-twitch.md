---
title: Slide Twitch
layout: post
tags: [python, llm, linux, twitch, obs]
date: 11 Oct 2023
comments: true
---

# About

Python tool that integrates with Twitch, OBS, OpenAI and FakeYou to generate
slideshow presentations.

<p align="center">
  <img src="/images/slide-twitch/slide-twitch.png" width="1000"/>
</p>

# Usage

You will need [OBS](https://obsproject.com/) with [VLC plugin
enabled](https://obsproject.com/forum/threads/obs-studio-how-to-get-vlc-video-source.72661/).
I use Arch **BTW**, so I needed to install
[obs-studio-git](https://aur.archlinux.org/packages/obs-studio-git). You will
also need to [enable
websockets](https://obsproject.com/forum/resources/obs-websocket-remote-control-obs-studio-using-websockets.466/)
in OBS.

Next you will need to make a copy of [.env.example](.env.example) in `.env` and
fill it in with your information.

- `OPENAI_API_KEY` => You get this one on your openai
  [account](https://platform.openai.com/account/api-keys), keep in mind that
  you need to pay to use this service.
- `FAKEYOU_USERNAME` & `FAKEYOU_PASSWORD` => These are from the
  <https://fakeyou.com/> website. Dodgy stuff, but it works. If you want speed
  you will need to buy the fastest one. Otherwise it will take a long time to
  generate TTS.
- `TWITCH_APP_ID` & `TWITCH_APP_SECRET` => You get this by going to the twitch
  dev console <https://dev.twitch.tv/console> and building a new app. When you
  fill in the details, for the OAuth Redirect URLs use
  `http://localhost:17563`. This is the address that the twitch client I use
  for Python uses <https://pytwitchapi.dev/en/stable/index.html#user-authentication>
- `TWITCH_TARGET_CHANNEL` => This is your channel name, dummy :)
- `OBS_WEBSOCKET_IP` & `OBS_WEBSOCKET_PORT` & `OBS_WEBSOCKET_PASSWORD` => These
  you get when you enable the websockets in OBS

After doing everything you can install the dependencies and run the project. I
started to become a [poetry](https://python-poetry.org/) enjoyer, so I would
recommend using it.

```console
poetry install
poetry run slide-twitch
```

# Conclusion

- [GitHub Repo](https://github.com/alexjercan/slide-twitch)
