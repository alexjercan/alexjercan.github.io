---
title: Neovim
layout: post
tags: [learn, practice, programming, linux]
date: 25 July 2023
comments: true
---

# About

Here you will learn how to configure neovim, a powerful and extensible text
editor. We will explore various plugins, keybindings, and configurations that
will smoothen your developer experience.

<p align="center">
  <img src="/images/neovim/lsp.png" width="1000"/>
</p>

# Install

### Neovim

The first thing you will need to do is install neovim. You can follow the steps
from the official
[documentation](https://github.com/neovim/neovim/wiki/Installing-Neovim).

> **_NOTE:_** One thing to
keep in mind is that for this configuration to work you will need at least
version `0.9.0`.

The easiest way to install neovim on Ubuntu is to go to the
[releases](https://github.com/neovim/neovim/releases) page on the official repo
and download the `nvim.appimage`. To get the latest version you can just run

```console
curl -LO https://github.com/neovim/neovim/releases/latest/download/nvim.appimage
```

After you download the appimage you need to make it executable by running

```console
chmod +x nvim.appimage
```

This will allow you to execute the program using `./nvim.appimage`. Now, run
`./nvim.appimage --version` in the terminal and make sure the version is at
least `0.9.0`. The next step is to move this binary in the PATH of the system
so you can execute it like any other tool (e.g `ls`, `cat` etc.)

```console
mv nvim.appimage /usr/bin/nvim
```

> **_NOTE:_** If you prefer to have the nvim binary in your home folder you can
use `~/.local/bin/nvim` as the target, but you will have to make sure that the
path is included in the PATH variable. You can check that with `echo $PATH`.

> **_NOTE:_** If you really want to use the package manager you can take a look
at <https://github.com/neovim/neovim/wiki/Installing-Neovim#ubuntu> and use the
_unstable_ ppa.

This is enough for you to get started working on editing some text. But you can
go down the rabbit hole of configuring your neovim experience first.

### Dependencies

To customize the neovim experience you will need some external tools first.

- [packer.nvim](https://github.com/wbthomason/packer.nvim)
- [ripgrep](https://github.com/BurntSushi/ripgrep)
- [fdfind](https://github.com/sharkdp/fd)

First one is packer.nvim. This is one of the many package managers for neovim.
And the one that I use.

Ripgrep is optional, but a good addition if you want to use
[telescope](https://github.com/nvim-telescope/telescope.nvim). `rg` is a `grep`
replacement. It is used to search for words in the given folder.

Fdfind is also optional, but it integrates well with telescope (again). It an
alternative to the `find` command. It is used to search for files in the given
folder. One thing to keep in mind is that the package name is called `fd-find`.
See <https://github.com/sharkdp/fd#on-ubuntu>.

> **_NOTE:_** `rg` and `fdfind` are tools that can be used normally in the
terminal. It is just that neovim works better when combined with tools that
allow you to search for files or words, so you don't have to use your mouse to
navigate and waste time.

# Configuration

If you just want an out of the box config with some sensible defaults you can
use my [nvim.dotfiles](https://github.com/alexjercan/nvim.dotfiles/tree/master)
repo.

You will just need to download the source code and run the install script. For example

```console
git clone git@github.com:alexjercan/nvim.dotfiles.git
cd nvim.dotfiles
./install
```

> **_NOTE:_** This will remove the configuration from `~/.config/nvim/` and create a symlink
to my repo, so make sure to backup your old configuration if you want to.

The configuration is written in the `lua` programming language.

### Packages

To install the packages you will have to open the `nvim/lua/neovim/packer.lua`
file with neovim.

```console
nvim nvim/lua/neovim/packer.lua
```

Then you will have to source the file using the command `:so %` inside the
editor. And then run the command `:PackerSync`. This should install all the
plugins listed in the packer.lua file.

- [telescope](https://github.com/nvim-telescope/telescope.nvim) Plugin to use
  when you want to search for words or files in your project.
- [nvim-treesitter](https://github.com/nvim-treesitter/nvim-treesitter) Plugin
  that integrates parsers into neovim. Good for syntax highlight.
- [nvim-treesitter-context](https://github.com/nvim-treesitter/nvim-treesitter-context)
  Plugin that shows you at the top of the file the scope (function, if, for,
  etc) that you are in.
- [harpoon](https://github.com/ThePrimeagen/harpoon) Plugin for navigating
  between a small number of files. Think of it as a `^` on steroids.
- [undotree](https://github.com/mbbill/undotree) Plugin that is great for when
  you like to Ctrl+z a lot (but in vim you use `u`).
- [vim-fugitive](https://github.com/tpope/vim-fugitive) Plugin that integrates
  git into neovim.
- [nvim-surround](https://github.com/kylechui/nvim-surround) Plugin that is
  nice to use when you want to surround pieces of text with `(["` etc. with as
  few keystrokes as possible.
- [zen-mode.nvim](https://github.com/folke/zen-mode.nvim) Plugin to make neovim
  look nicer.
- [trouble.nvim](https://github.com/folke/trouble.nvim) Plugin that shows all
  the diagnostics, searches, etc. in a list you can navigate with `hjkl`.
- [lsp-zero.nvim](https://github.com/VonHeikemen/lsp-zero.nvim) Plugin that
  makes setting up LSP really easy.
- [rose-pine/neovim](https://github.com/rose-pine/neovim) Color scheme that I
  use. You can use anything you want here.
- [github/copilot.vim](https://github.com/github/copilot.vim) Copilot for
  neovim. Not mandatory to use, since it requires a paid subscription.
- [codehint](https://github.com/alexjercan/codehint) Plugin that I wrote and
  uses ChatGPT to give code hints. Not mandatory to use since it requires
  access to ChatGPT api.

This showcases the plugins that I have installed by default. If you do not want
to use one of them you can just remove it from the `packer.lua` file. Or if you
want to add a new plugin you can just add it using the convention from the
packer documentation.

### Sets

This section contains the sets that I use throughout my development.

```lua
vim.opt.guicursor = ""          -- this setting makes it so that the cursor
                                -- will always look like a filled block.

vim.opt.nu = true               -- enable line numbers
vim.opt.relativenumber = true   -- enable relative line numbers

vim.opt.errorbells = false      -- disables the anoying error bells.

-- tab stuff, I don't understand it,
-- but it works and it gives me tabs made up of 4 spaces.
vim.opt.tabstop = 4
vim.opt.softtabstop = 4
vim.opt.shiftwidth = 4
vim.opt.expandtab = true

vim.opt.smartindent = true      -- try to indent when you can (e.g in Python)

vim.opt.wrap = false            -- do not wrap the text

vim.opt.swapfile = false        -- do not create a swap file (what is this, Microsoft?)
vim.opt.backup = false          -- no backup

-- options for the undotree plugin
vim.opt.undodir = os.getenv("HOME") .. "/.vim/undodir"
vim.opt.undofile = true

vim.opt.hlsearch = false        -- does not highlight all the found terms
vim.opt.incsearch = true        -- turns on incremental search

vim.opt.termguicolors = true    -- more colors

vim.opt.scrolloff = 8           -- the offset you can do `jk` until it starts to scroll the screen
vim.opt.signcolumn = "yes"      -- column for signs, like error messages, warnings, etc.
vim.opt.isfname:append("@-@")   -- nerd glasses

vim.opt.cmdheight = 1           -- height of the command bar at the bottom

-- Having longer updatetime (default is 4000 ms = 4 s) leads to noticeable
-- delays and poor user experience.
vim.opt.updatetime = 50

-- Don't pass messages to |ins-completion-menu|.
vim.opt.shortmess:append("c")

vim.opt.colorcolumn = "80"      -- adds a vertical column at 80 characters to make you
                                -- rethink that crazy 5 for loop function

vim.g.mapleader = " "           -- to use space in some keybindings later
```

Feel free to copy paste this if you only want some sets in you nvim config. You
can find this in the `nvim/lua/neovim/set.lua` file.

### Keybindings

The following keybindings are just better defaults to use.

```lua
-- open the file viewer with space+p+v
vim.keymap.set("n", "<leader>pv", vim.cmd.Ex)

-- cool move line up or down in visual mode
vim.keymap.set("v", "J", ":m '>+1<CR>gv=gv")
vim.keymap.set("v", "K", ":m '<-2<CR>gv=gv")

-- don't ask (copy line from current position to the end)
vim.keymap.set("n", "Y", "yg$")

-- better moves
vim.keymap.set("n", "J", "mzJ`z")
vim.keymap.set("n", "<C-d>", "<C-d>zz")
vim.keymap.set("n", "<C-u>", "<C-u>zz")
vim.keymap.set("n", "n", "nzzzv")
vim.keymap.set("n", "N", "Nzzzv")

-- greatest remap ever (paste without overwritting the yank register)
vim.keymap.set("x", "<leader>p", [["_dP]])

-- next greatest remap ever : asbjornHaland
vim.keymap.set({"n", "v"}, "<leader>y", [["+y]])
vim.keymap.set("n", "<leader>Y", [["+Y]])

-- real alphas always delete in main register, I am not an alpha
vim.keymap.set({"n", "v"}, "<leader>d", [["_d]])

-- This is going to get me cancelled
vim.keymap.set("i", "<C-c>", "<Esc>")

-- never press Q
vim.keymap.set("n", "Q", "<nop>")

-- run a formatter if you have one
vim.keymap.set("n", "<leader>f", vim.lsp.buf.format)

vim.keymap.set("n", "<C-k>", "<cmd>cnext<CR>zz")
vim.keymap.set("n", "<C-j>", "<cmd>cprev<CR>zz")
vim.keymap.set("n", "<leader>k", "<cmd>lnext<CR>zz")
vim.keymap.set("n", "<leader>j", "<cmd>lprev<CR>zz")

-- for fast repalce
vim.keymap.set("n", "<leader>s", [[:%s/\<<C-r><C-w>\>/<C-r><C-w>/gI<Left><Left><Left>]])

-- do this on the bash script you download from the internet
vim.keymap.set("n", "<leader>x", "<cmd>!chmod +x %<CR>", { silent = true })

-- for when you have to share the screen and they ask you
-- "what does the code from the line 10 mean?"
vim.keymap.set("n", "<leader>rnu", ":set rnu!<CR>")

-- don't be a b**, just use hjkl
vim.keymap.set("i", "<Up>", "<C-o>:echom \"--> k <-- \"<CR>")
vim.keymap.set("i", "<Down>", "<C-o>:echom \"--> j <-- \"<CR>")
vim.keymap.set("i", "<Right>", "<C-o>:echom \"--> l <-- \"<CR>")
vim.keymap.set("i", "<Left>", "<C-o>:echom \"--> h <-- \"<CR>")
vim.keymap.set("n", "<Up>", ":echom \"--> k <-- \"<CR>")
vim.keymap.set("n", "<Down>", ":echom \"--> j <-- \"<CR>")
vim.keymap.set("n", "<Right>", ":echom \"--> l <-- \"<CR>")
vim.keymap.set("n", "<Left>", ":echom \"--> h <-- \"<CR>")
```

Again, these keybindings are not really related to any plugins, and you can use
or change any of them. You can find this file in `nvim/lua/neovim/remap.lua`.

If you want to check some keybindings for specific plugins you can check out
the `after/plugin` folder and look for which plugin you want to see the
keybinds for.

### LSP

For the LSP I am using `lsp-zero` which creates a seamless experience when
interacting with lsp servers. To easily install a new lsp you can run the
`:Mason` command. This will open up a menu where you can download and install
lsp for any language. There are some downsides to this method, because you
might not always find the latest versions. But it is a lot easier than having
to manually install them (unless the programming language has a really good
build tool/package manager).

# Conclusion

Feel free to use the github repo and make changes to it as you wish.

- [GitHub Repo](https://github.com/alexjercan/nvim.dotfiles)
