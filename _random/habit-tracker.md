---
title: HabitTracker
layout: post
tags: [learn, practice, programming, svelte, javascript, firebase, vercel]
date: 14 Aug 2023
comments: true
---

# About

Habit Tracker is a web app I build to be able to keep track of daily habits
more easily. Before this app I was using obsidian/notion. However I felt like
it was easier to have a table on a piece of paper. So I wanted to implement a
nicer looking piece of paper in the browser.

# Usage

The application has 2 pages.

- `/` First the dashboard, which shows you the status of the daily habits.
- `/account` The account page, where you can add/remove/edit daily habits from
  the list.

Initially the account comes with no daily habits. So you will have to come up
with your own. I am not sure if I should add some default ones, or do a
tutorial, so here are some examples I use:

- `🧼 Bathroom`
- `📕 Read`
- `💪 Gym`
- `🏃 Cardio`

# Technologies

To implement this app I have used the FKIT stack.

The application is built using [sveltekit](https://kit.svelte.dev/).

The application uses [firebase](https://firebase.google.com/) to handle
authentication and database storage.

For deployment I have used [vercel](https://vercel.com/).

### Limitations

Right now, the tools looks kind of bad on mobile if you have a lot of daily
tasks.

# Conclusion

This web app is great if you want to have a nice looking table with the status
of your daily habits.

- [GitHub Repo](https://github.com/alexjercan/habit-tracker)
- [Website](https://habit-tracker-tan.vercel.app/)
