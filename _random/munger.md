---
title: Munger
layout: post
tags: [security, rust, linux]
date: 9 Oct 2023
comments: true
---

# About

Munger is a CLI tool written in Rust that will apply common patterns on a list
of passwords to generate more similar passwords.

The supported patterns are:
- leet speak: will change different letters to special characters or digits (e.g e -> 3)
- capitalization: will attempt to use lower letters, upper case and capitalized words
- suffix: will add from a list of common suffixes to the word from the list

# Usage

To run the tool you will want to give it the word list using the `-w` flag
followed by the name of the file. And specify the output file. In case `-w` or
`-o` are missing, `stdin` and `stdout`, respectively, will be used.

```console
cargo run -- -w wordlist.txt -o passwords.txt
```

# Conclusion

- [GitHub Repo](https://github.com/alexjercan/munger)
