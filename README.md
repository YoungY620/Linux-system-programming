# Linux-system-programming
including exercises of Linux system call, network programming, and an assignment of a network game

# Useage
To try out the game:

```bash
cd assignment3
cc server3.c -o server
./server 8080
```

PS：请不要在后台运行 server，因为会有用于观察行为的日志输出

Before compile the client, install "curses"

```bash
sudo apt install lib32ncurses5-dev
```

run clients in another two terminals:

```bash
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

这个项目仅供本人分享和留档使用 （意思就是别抄袭）

引用老师原话:
>"各位同学，已非常确定有若干名同学拿学长的大作业直接给我——都说了我们这个游戏作业特有个性，很容易就能发现——一个是飞机，另外一个是吞食蛇（我今天晚上直接找到了该四年级同学，让他把网上资源的撤下了）。因此，也请各位同学不要把你的作业放到网上（包括GitHub）。这个第一是在坑你的学弟学妹，第二是在浪费我的脑细胞。"  
