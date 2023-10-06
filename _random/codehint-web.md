---
title: CodeHint Webapp
layout: post
tags: [webdev, llm, svelte, javascript, firebase, vercel]
date: 2 Sep 2023
comments: true
---

# About

The CodeHint Webapp is part of the CodeHint plugin I implemented for NeoVim
[CodeHint](/random/codehint). The goal of this project is to make a simple web
interface for ChatGPT and Llama2 models to suggest hints for code. And to make
it easy for other developers to make API based extensions for other editors.

<p align="center">
  <img src="/images/codehint-webapp/01-example.png" width="1000"/>
</p>

# Usage

The application has 3 pages.

- `/` First the dashboard, which shows you the code editor and the hints.
- `/account` The account page, where you can logout of the application or
  generate an API key.
- `/credits` The credits page, where you can buy credits to use the
  application.

Initially you will get 10 free hints. You can buy 100 credits for $1 and each
time you use the hint generation you use 1 credit.

<style>
  #wrap {
    width: 100%;
    height: 650px;
    overflow: hidden;
  }
  #scaled-frame {
    width: 1408px;
    height: 896px;
    transform: scale(0.70);
    transform-origin: 0 0;
  }
</style>

<div id="wrap">
    <iframe
        id="scaled-frame"
        scrolling="no"
        title="Habit Tracker"
        src="https://codehint-app.vercel.app/"
        frameborder="0"
    >
    </iframe>
</div>

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
- [Website](https://codehint-app.vercel.app/)
