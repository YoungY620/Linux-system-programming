/*
    实现了数据的传递、通信的各个阶段，只差游戏逻辑和图形界面
 */
#include <netinet/in.h>     // for sockaddr_in
#include <sys/types.h>      // for socket
#include <sys/socket.h>     // for socket
#include <stdio.h>          // for printf
#include <stdlib.h>         // for exit
#include <string.h>         // 
#include <unistd.h>         // read
#include <netinet/in.h>     // inet_addr();
#include <arpa/inet.h>      // inet_addr();
#include <sys/epoll.h>      // epoll
#include <fcntl.h>          //fcntl F_GETFL F_SETFL
#include <termios.h>

#include "data.h"

struct termios init_tty, new_tty;
enum client_phase{
    init, conn_wait, play, close_wait, closed
};

void setnonblocking(int fd) {
	int opts;
	opts=fcntl(fd, F_GETFL);
	if(opts<0) {
		perror("fcntl(sock,GETFL)");
		exit(1);
	}
	opts = opts|O_NONBLOCK;
	if(fcntl(fd, F_SETFL, opts)<0) {
		perror("fcntl(sock,SETFL,opts)");
		exit(1);
	}
}

void init_client_socket(int *sock_fd, char ip[], char portnumber[]){
    struct sockaddr_in addr;

    (*sock_fd) = socket(AF_INET, SOCK_STREAM,0);
    if((*sock_fd) == -1){
        perror("socket");
        exit(1);
    }
    
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip);
    addr.sin_port = htons(atoi(portnumber));
    if(-1 == connect(*sock_fd, (const struct sockaddr *)&addr, sizeof(struct sockaddr_in))){
        perror("connect");
        exit(1);
    }
}

void decode(char *buf, struct status *sts){
        char lenstr[10];

        sscanf(buf,"%s",lenstr);
        strcpy(buf,buf+strlen(lenstr));

        sts->bull_size = atoi(lenstr);
        sts->bullets = malloc(sizeof(struct pos)*sts->bull_size);
        str_to_data(buf, sts, (sts->bull_size));
}

void init_epoll(int *epollfd, int listen_sock){
	struct epoll_event ev;

	*epollfd = epoll_create(10);
	if ((*epollfd) == -1) {
		perror("epoll_create");
		exit(EXIT_FAILURE);
	}

	ev.events = EPOLLIN;
	ev.data.fd = listen_sock;
	setnonblocking(listen_sock);
	if (epoll_ctl(*epollfd, EPOLL_CTL_ADD, listen_sock, &ev) == -1) {
		perror("epoll_ctl: listen_sock");
		exit(EXIT_FAILURE);
	}

	ev.events = EPOLLIN;
	ev.data.fd = 0;
	setnonblocking(0);
	if (epoll_ctl(*epollfd, EPOLL_CTL_ADD, 0, &ev) == -1) {
		perror("epoll_ctl: tty");
		exit(EXIT_FAILURE);
	}
    tcgetattr(0,&init_tty);
    tcgetattr(0,&new_tty);
    new_tty.c_lflag &= ~(ECHO|ICANON);
    new_tty.c_cc[VMIN]=1;
    new_tty.c_cc[VTIME]=0;
    tcsetattr(0,TCSAFLUSH,&new_tty);
}

void send_opt(int fd, int sock_fd, enum client_phase *phase){
    char ttybuf;
    int ntty = read(fd, &ttybuf, 1);
    if (ntty == -1){
        perror("tty read");
        exit(EXIT_FAILURE);
    }
    printf("%c %d\n",ttybuf,(int)ttybuf);
    if(*phase == play){
        if (ttybuf == 'a' || ttybuf == 'A'){
            write(sock_fd, "OPT L", 5);
        } else if (ttybuf == 's' || ttybuf == 'S'){
            write(sock_fd, "OPT D", 5);
        } else if (ttybuf == 'd' || ttybuf == 'D'){
            write(sock_fd, "OPT R", 5);
        } else if (ttybuf == 'w' || ttybuf == 'W'){
            write(sock_fd, "OPT U", 5);
        } else if (ttybuf == 'q' || ttybuf == 'Q'){
            write(sock_fd, "OPT Q", 5);
        }
    }
}

void std_exit(int flg, int sock_fd, int epollfd){
    close(sock_fd);
    close(epollfd);
    init_tty.c_cc[VMIN]=1;
    tcsetattr(0,TCSAFLUSH,&init_tty);
    close(0);   // 一定要在设置完tssetattr之后再close，否则设置无效
    exit(flg);
}

void handle_msg(int sock_fd, int epollfd, enum client_phase *phase){
    char buf[1024];
    int nread;
    char cmd[5],content[1024];
    memset(buf,sizeof(buf),' ');

    // new status or exit
    if(-1==(nread = read(sock_fd,buf,sizeof(buf)))){
        perror("read");
        std_exit(EXIT_FAILURE,sock_fd,epollfd);
    }
    // 服务端主动断开，close并退出
    if(nread == 0){
        fprintf(stderr,"server is out of connection\n");
        std_exit(EXIT_FAILURE,sock_fd,epollfd);
    }
    sscanf(buf,"%s%s",cmd,content);
    strcpy(content,buf+strlen(cmd)+1);
    // possible cmd:
    // WAIT 忽略tty输入(phase = conn_wait)直到接到第一个POST
    // POST 解除wait状态，重绘场景。
    // EXIT 退出，
    if(strcmp(cmd, "WAIT") == 0){
        *phase = conn_wait;
        printf("wait %s %s\n",cmd, content);
    }else if(strcmp(cmd, "EXIT") == 0){
        *phase = closed;
        printf("exit %s %s\n",cmd, content);
    }else if(strcmp(cmd,"POST") == 0){
        if(*phase == conn_wait) 
            *phase = play;
        printf("post %s %s\n",cmd, content);
    }
}

int main(int argc, char **argv){
    int sock_fd;
    struct status sts;
    int readn;
    int epollfd;
    struct epoll_event events[2];
    struct termios tty;
    enum client_phase phase = init; 
    //states: 0-init,1-connection wait,2-play,3-close wait,4-close

    if(argc < 3){
        fprintf(stderr, "usage: not enough args. Needed: ip, portnumber");
        exit(EXIT_FAILURE);
    }
    init_client_socket(&sock_fd, argv[1], argv[2]);
    //regist 2 fds: socketfd and keyboard
    init_epoll(&epollfd, sock_fd);

    while(phase != closed){
        int nfds = epoll_wait(epollfd, events,2,-1);
        int i;

        if(nfds == -1){
            perror("epoll_wait");
            std_exit(EXIT_FAILURE,sock_fd,epollfd);
        }
        for(i=0;i<nfds;i++){
            int fd = events[i].data.fd;
            if(fd == 0){
                //tty 
                send_opt(fd, sock_fd, &phase);
            }else {
                handle_msg(sock_fd,epollfd, &phase);
            }
        }
    }

    std_exit(EXIT_SUCCESS,sock_fd,epollfd);
}