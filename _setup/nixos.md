---
title: My Developer Experience
layout: post
tags: [learn, linux, neovim, tmux, dotfiles, nixos]
date: 02 July 2024
comments: true
---

# About

My developer setup for neovim and i3 using NixOS 🤓.

<div class="video-container" align="center">
	<iframe
        title="YouTube video player"
        width="840"
        height="478"
        src="https://www.youtube.com/embed/ks0V7qxXXbA"
        frameborder="0"
        allow="accelerometer; autoplay; clipboard-write; encrypted-media; gyroscope; picture-in-picture; web-share"
        allowfullscreen
    >
    </iframe>
</div>

<p align="center">
  <img src="/images/nixos-setup/desktop.png" width="1000"/>
</p>

# Quickstart

To install my main system config you can run

```console
sudo nixos-rebuild switch --flake .#main
```

To install my home manager config (probably this is what you want, for
dotfiles; will probably need to update user name and some other stuff)

```console
home-manager switch --flake .#alex
```

# Conclusion

Feel free to use the github repo and make changes to it as you wish. There is
also more to come, and if you have any questions you can leave them in the
comments.

- [GitHub Repo](https://github.com/alexjercan/nix.dotfiles)

