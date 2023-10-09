---
title: Rust TODO TUI
layout: post
tags: [linux, rust]
date: 9 Oct 2023
comments: true
---

# About

_Rust TODO TUI_ is a simple terminal UI application for daily tasks.

<p align="center">
  <img src="/images/rust-todo-tui/todo.png" width="1000"/>
</p>

# Usage

### Quickstart

To run the application you just need to compile the program and it will handle
all the default configs.

```console
cargo run
```

### Modes

By default, the application will start in TUI mode, but you can also visualize
statistics of your tasks using sub-commands.

- `status` this sub-commands will display the number of tasks done out of the
  total, for example `2/8`.
- `details` this sub-command will display the list of items to stdout as
  Markdown

### Config

The tools will look for a configuration file in `XDG_CONFIG_HOME`. It will
search for `todo/config.json`. If it cannot find the file, it will use the
default settings.

You can specify the path to a custom config file using the `-c/--config`
argument.

The configuration file is in JSON format, and it has the following properties.

- `path`: The path to the task's directory. This is where all the markdown
  files will be saved and loaded from. By default, it will be set to
  `$HOME/.config/todo/`.
- `date_format`: The format of date that will be used to name the files and use
  as a display title in the TUI. By default, it is `%Y-%m-%d`.
- `habits`: A list of custom items that will be prepended to each task file on
  creation. By default, it will be an empty list `[]`.

Configuration example

```json
{
    "path": "/home/alex/personal/todo",
    "date_format": "%Y-%m-%d",
    "habits": [
        "🧼 Morning Routine",
        "📕 Read",
        "💪 Gym",
        "✍️ Journal",
    ]
}
```

# Conclusion

- [GitHub Repo](https://github.com/alexjercan/rust-todo-tui)

