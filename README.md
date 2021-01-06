# Linux-system-programming
including exercises of Linux system call, network programming, and an assignment of a network game

# Useage
To try out the game:
```
cd assignment3
cc server3.c -o server
./server 8080
```
Before compile the client, install "curses"
```
sudo apt install lib32ncurses5-dev
```
another stupid but helpful way for CentOS:
```
yum install *curses*
```
run clients in another two terminals:
```
cd assignment3
cc client3.c -o client -lcurses
./client 127.0.0.1 8080
```
Following is the image if succeed :
```
|__HP:2__|
   |02|

                            *
                                *
                         __|02|__
                        |__HP:2__|
```
It is designed to control with "WSAD", and shoot with space.

# Big Warnning!
If you're an undergraduate of NEU and your professor is Shi Kai, do not copy this assignment!(手动狗头)