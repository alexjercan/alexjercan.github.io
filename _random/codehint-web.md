---
title: CodeHint Webapp
layout: post
tags: [learn, practice, programming, webdev, generative ai, llm]
date: 2 Sep 2023
comments: true
---

# About

The CodeHint Webapp is part of the CodeHint plugin I implemented for NeoVim
[CodeHint](/random/codehint). The goal of this project is to make a simple web
interface for ChatGPT and Llama2 models to suggest hints for code. And to make
it easy for other developers to make API based extensions for other editors.

# Usage

The application has 3 pages.

- `/` First the dashboard, which shows you the code editor and the hints.
- `/account` The account page, where you can logout of the application.
- `/credits` The credits page, where you can buy credits to use the
  application.

Initially you will get 10 free hints.

# Technologies

To implement this app I have used the FKIT stack.

The application is built using [sveltekit](https://kit.svelte.dev/).

The application uses [firebase](https://firebase.google.com/) to handle
authentication and database storage.

For deployment I have used [vercel](https://vercel.com/).

# Conclusion

This webapp is a good alternative to the neovim plugin I developed, or if you
prefer to use the browser.

- [GitHub Repo](https://github.com/alexjercan/codehint-app)
