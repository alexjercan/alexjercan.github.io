---
title: My Desktop Experience
layout: post
tags: [learn, linux, hyprland, dotfiles]
date: 02 October 2023
comments: true
---

# About

Here you will learn how to configure neovim, a powerful and extensible text
editor. We will explore various plugins, keybindings, and configurations that
will smoothen your developer experience. Then we will look into how I set up my
Tmux to achieve maximum efficiency.

Here you will learn how to configure
[hyprland](https://github.com/hyprwm/Hyprland), a power and highly customizable
dynamic tiling Wayland compositor. I will also include utilities such as a
status bar, notification daemon, a terminal emulator, rofi and others.

<p align="center">
  <img src="https://raw.githubusercontent.com/alexjercan/hyprland.dotfiles/master/.resources/neofetch.png" width="1000"/>
</p>

# Install

The project is built such that it makes installing the dotfiles and the
dependencies effortlessly.

First clone the repository containing the configuration files.

```console
git clone git@github.com:alexjercan/hyprland.dotfiles.git
```

Then run the `install` script.

```console
cd hyprland.dotfiles
./install
```

> **_NOTE:_** Running the `install` script will delete your old dotfiles for
the applications included in this repo. Make a backup if you don't want to lose
them and always check the `sh` scripts before running them.

> **_NOTE:_** The `install` script is still WIP and is not finished yet. Report
any bugs you encounter to the
[issues](https://github.com/alexjercan/hyprland.dotfiles/issues) page.

# Tips and Tricks

- To check the keybindings use `SUPER` + `backspace`. It will open up a rofi
menu with all the configured keybindings. You can also change them in
`~/.config/hypr/hyprland.conf`.

# Troubleshooting

TODO

# Conclusion

Feel free to use the github repo and make changes to it as you wish.

- [GitHub Repo](https://github.com/alexjercan/hyprland.dotfiles)
