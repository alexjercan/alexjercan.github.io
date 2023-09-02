---
title: Lesson Learned?
layout: post
tags: [learn, practice, programming, linux, ctf, sqli, tryhackme]
date: 2 Sep 2023
comments: true
---

# About

Lesson Learned? is a CTF hosted on TryHackMe
[here](https://tryhackme.com/room/lessonlearned).

This is a CTF that has some enumeration and SQLi elements.

# Walkthrough

I usually like to start by doing an `export IP=...` so that I don't have to
remember the IP of the target all the time.

Next I have started the usual nmap scan with

```console
nmap $IP
```

<p align="center">
  <img src="/images/lessonlearned/01-nmap-scan.png" width="1000"/>
</p>

From the CTF description we could have already figured out that we only have to
deal with a http server, but it is never a bad thing to check for extra attack
surface.

We can navigate to `http://MACHINE_IP` and check what the website is about.

<p align="center">
  <img src="/images/lessonlearned/02-login-screen.png" width="1000"/>
</p>

On the main page of the website we find a login screen with a username and
password field. If we randomly try some username and password combinations,
like the usual `admin:admin` we find an error message _Invalid username and
password._

From past experience, I taught that if we find the right username we might get
a different error message. To validate this idea we can try to use
[hydra](https://github.com/vanhauser-thc/thc-hydra) with a custom username list
from [seclists](https://github.com/danielmiessler/SecLists).

```console
hydra -L /usr/share/seclists/Usernames/xato-net-10-million-usernames.txt -p password $IP http-post-form "/:username=^USER^&password=^PASS^:Invalid username and password."
```

This will setup hydra to use the xato-net-10-million-usernames usernames; for
the password we will use the string _password_ since we do not care about the
password; for the address we will our the IP address of the machine; next,
hydra will require the protocol to use, which is http post form, and finally we
have to give it the path `/` the form structure, which we can get by looking
the network tab of any request with invalid credentials and the error message
that appears on wrong auth.

<p align="center">
  <img src="/images/lessonlearned/04-hydra-result.png" width="1000"/>
</p>

With this command we found an username that has a different error message `martin`.

Next we can try to build a SQLi prompt.

We can try to think of the SQL query as

```SQL
SELECT * FROM users WHERE username = 'username' AND password = 'password'
```

Since we know the username we can try to inject into the query a string that
will work regardless of the password (this works if the password is checked
after the username only). We would like to achieve something like

```SQL
SELECT * FROM users WHERE username = 'martin'
```

To achieve this we will have to provide a comment in the username field `--`.
If we use the following text as the username we will remove the password check

```text
martin' -- -
```

```SQL
SELECT * FROM users WHERE username = 'martin' -- -' AND password = ''
```

With this new prompt we bypass the login and find the flag.

<p align="center">
  <img src="/images/lessonlearned/06-flag.png" width="1000"/>
</p>

# Conclusion

Straightforward challenge with interesting solution.
