[SIGMSG](https://github.com/JHeflinger/sigmsg) - Cross-platform P2P C Direct Messaging Platform
============================================================================

## INFO

Currently SIGMSG is at v0.1! It supports direct messaging as long as both clients are online. There may still be some bugs
depending on what kind of NAT or firewalls you have, but overall it works pretty consistently, even with peers across the country!
There's a lot more planned in the future when I have more time to work on this, but the ideal end goal is something akin to a 
terminal based discord-like application thats p2p!

## REQUIREMENTS

There are no requirements! (I think) Everything is done via C sockets and system calls, as well as ncurses/PDcurses for terminal
rendering.

## BUILDING

First ensure you have cloned the repo along with any subrepos.
```
git clone https://github.com/JHeflinger/SIGMSG.git --recursive
cd SIGMSG
```
If you have already cloned it, you can also download the subrepos by running the following in the repo's working directory:
```
git submodule update --recursive --init
```
If you're on Linux, you can compile and run the program using `run.sh`
```
./run.sh
```
If you're on Windows, you can compile and run the program using `run.bat`
```
./run.bat
```

> **_NOTE:_**  SIGMSG is only cross-platform for Linux and Windows systems - Mac is not supported.