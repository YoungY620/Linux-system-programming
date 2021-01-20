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
If you're an undergraduate of NEU and your professor is SK, do not copy this assignment!(手动狗头)  
引用老师原话:
>"各位同学，已非常确定有若干名同学拿学长的大作业直接给我——都说了我们这个游戏作业特有个性，很容易就能发现——一个是飞机，另外一个是吞食蛇（我今天晚上直接找到了该四年级同学，让他把网上资源的撤下了）。因此，也请各位同学不要把你的作业放到网上（包括GitHub）。这个第一是在坑你的学弟学妹，第二是在浪费我的脑细胞。"  
## 清华大学学生纪律处分管理规定实施细则  
身不能往，心向往之，各位自勉吧  

第六章　学术不端、违反学习纪律的行为与处分 

第二十一条 有下列违反课程学习纪律情形之一的，给予警告以上、留校察看以下处分：  
（一）课程作业抄袭严重的；  
（二）实验报告抄袭严重或者篡改实验数据的；  
（三）期中、期末课程论文抄袭严重的；  
（四）在课程学习过程中严重弄虚作假的其他情形。
