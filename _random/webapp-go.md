---
title: TOP G WebApp
layout: post
tags: [go, docker, tailwind, css, postgres, generative ai, javascript]
date: 2 Jul 2024
comments: true
---

# About

This is a webapp that uses the TOP G stack (Tailwind, Ollama, Postgres, Golang).

TOP G allows you to create courses, add documents in those courses, and then
use LLMs to retrieve information from them.

You can watch the entire playlist of me building this thing live on YouTube
<https://www.youtube.com/watch?v=AGEVNDHAAXo&list=PLwHDUsnIdlMwFtstxyy2aroUbGCkTrxMd>.

# Quickstart

You will need to define the config.yaml and the secrets first. (They will be
kept in the repo for teaching purposes, just copy paste the example config and
fill in the github oauth stuff)

```console
docker-compose up
# open browser at `localhost:8080`
```

# Conclusion

- [GitHub Repo](https://github.com/alexjercan/webapp-go)


