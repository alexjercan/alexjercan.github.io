---
title: Tmux
layout: post
tags: [learn, practice, programming, linux]
date: 25 July 2023
comments: true
---

# About

Here you will learn how to configure tmux, a powerful and extensible terminal
multiplexer. We will explore my setup and look at some keybindings that I find
to improve my productivity.

<p align="center">
  <img src="/images/tmux/tmux.png" width="1000"/>
</p>

- [GitHub Repo](https://github.com/alexjercan/tmux.dotfiles)

In my opinion, you do not notice that much of a difference from just a
screenshot. Maybe you will see that there is a cool status bar, for which I use
a custom plugin instead of the default one. However the power of tmux comes
when you need to switch between multiple terminals, even multiple sessions.

# Install

### Tmux

The first thing you will need to do is install tmux. You can follow the steps
from the official
[documentation](https://github.com/tmux/tmux#installation). Most probably you
will find tmux available in your package manager (e.g `sudo apt install tmux`).

This is enough to get started using tmux with my config.

```console
git clone git@github.com:alexjercan/tmux.dotfiles.git
cd tmux.dotfiles
./install
```

However, I have also included a script that can be used to start a new session
for one of your projects really fast. Enter `tmux-sessionizer`.

### Tmux Sessionizer

This step is optional, but it gives you a great tool for working with tmux.

To be able to use sessionizer you need to install
[fzf](https://github.com/junegunn/fzf). This is a tool that allows you to
filter for files from the CLI.

Sessionizer works by first choosing a project folder. In my case I keep the
projects in `~/Documents/`, so I hardcded in the bash script

```bash
selected=$(find ~/Documents -mindepth 1 -maxdepth 1 -type d | fzf)
```

but you can change the target of the `find` command (you can even use multiple
folders, and change the depth). The important thing is to use fzf to quickly go
through projects and filter for the one that you want to open.

If you close the terminal but do not stop your tmux session, the next time you
use sessionizer it will open your project back up (if you search for it with
fzf). You can even create a new session from inside an existing session and
easily switch between the projects. (Altough for switching between sessions I
would rather use the tmux way of prefix+s, but you would first need to create
that session somehow, and sessionizer is really good at that)

> **_NOTE:_** My install script creates a symlink for tmux-sessionizer inside
`~/.local/bin/tmux-sessionizer`, so you might need to add `~/.local/bin/` to
PATH to make sure you can use it.

### Nerd Font

This step is optional. It is useful if you care about having nice icons in the
status bar.

You will have to install a [nerd font](https://www.nerdfonts.com/). I think
that any font works, but I am using `3270 Nerd Font` just for reference.

Again, to install the font, I just downloaded the zip archive and moved the
font files to `~/.local/share/fonts/`. There are many different ways to do it
and you can read more about it on the nerd fonts website.

### Tpm

If you want to install plugins you will need
[tpm](https://github.com/tmux-plugins/tpm). Don't forget to run `tmux source
~/.tmux.conf` once you are done with the installation. And after that you will
have to install all the plugins with `prefix+I` (prefix+shift+i basically)

The plugin that I use for the status bar is
[rose-pine/tmux](https://github.com/rose-pine/tmux) It is the same theme I use
for my [neovim](/setup/neovim) config.

Another plugin I use is
[tmux-sensible](https://github.com/tmux-plugins/tmux-sensible), which contains
some better default settings.

# Config

So that you have more colors (if your terminal allows it)

```conf
# terminal stuff
set -ga terminal-overrides ",screen-256color*:Tc"
set-option -g default-terminal "screen-256color"
set -s escape-time 0
```

Change prefix key to Ctrl-Space. I tried different prefix combinations, this
one stuck with me because it is similar to my neovim leader key (space).

```conf
unbind C-b
set -g prefix C-Space
bind C-Space send-prefix
```

By default tmux will have its first window set on `0`, but that makes it more
difficult to switch to it. If it starts at `1`, you will just have to do
`ctrl+space+1` -> `ctrl+space+2` etc to move between windows.

```conf
set -g base-index 1
set -g pane-base-index 1
set-window-option -g pane-base-index 1
set-option -g renumber-windows on
```

Set vi mode, so we can make use of `hjkl` and other good vim stuff.

```conf
set-window-option -g mode-keys vi
bind -T copy-mode-vi v send-keys -X begin-selection
bind -T copy-mode-vi y send-keys -X copy-pipe-and-cancel 'xclip -in -selection clipboard'
```

If you are into using panes I have also added some config for that. But I think
that just using windows is easier. This config will allow you to move between
panes with vim keybindings. The same you would move between vim windows.

```conf
bind -r ^ last-window
bind -r k select-pane -U
bind -r j select-pane -D
bind -r h select-pane -L
bind -r l select-pane -R
```

Again stuff for panes. If you open a pane it will do it in the current
directory.

```conf
bind '"' split-window -v -c "#{pane_current_path}"
bind % split-window -h -c "#{pane_current_path}"
```

This uses the sessionizer script to start a new session. If you want to open up
a new project just press `ctrl+space+f` and it will allow you to search in your
Documents folder.

```conf
bind-key -r f run-shell "tmux neww ~/.local/bin/tmux-sessionizer"
```

Useful when you add plugins and stuff.

```conf
bind r source-file ~/.tmux.conf
```

When you detach you go to your last open session. If no session exists it will
exit.

```conf
set-option -g detach-on-destroy off
```

Some config for my theme

```conf
set -g @rose_pine_variant 'main' # Options are 'main', 'moon' or 'dawn'
set -g @rose_pine_host 'off' # Enables hostname in the status bar
set -g @rose_pine_date_time '%H:%M | %a' # It accepts the date UNIX command format (man date for info)
set -g @rose_pine_window_tabs_enabled 'on' # Enables a symbol to separate window number from window name
set -g @rose_pine_left_separator ' > ' # The strings to use as separators are 1-space padded
set -g @rose_pine_right_separator ' < ' # Accepts both normal chars & nerdfont icons
set -g @rose_pine_field_separator ' | ' # Again, 1-space padding, it changes with prefix + I
```

Lastly you will need to add the tpm run line if you want to use plugins

```conf
run '~/.tmux/plugins/tpm/tpm'
```

Feel free to use my config file from the repo and change what you like.
