/*
    implement receive(&send) and decode a frame from server
    
    // possible cmd:
    // WAIT 忽略tty输入(phase = conn_wait)直到接到第一个POST
    // POST 解除wait状态，重绘场景。
    // EXIT 退出，
 */
#include <netinet/in.h>     // for sockaddr_in
#include <sys/types.h>      // for socket
#include <sys/socket.h>     // for socket
#include <stdio.h>          // for printf
#include <stdlib.h>         // for exit
#include <string.h>         // for bzero
#include <unistd.h>         // read
#include <netinet/in.h>     // inet_addr();
#include <arpa/inet.h>      // inet_addr();
#include <sys/epoll.h>      // epoll
#include <fcntl.h>          // fcntl F_GETFL F_SETFL
#include <termios.h>
#include <sys/stat.h>
#include<time.h>
#include <errno.h>

#define FILE_LOG 1

#include "data.h"
#include "graph.h"

struct termios init_tty, new_tty;
enum client_phase{
    init, conn_wait, play, close_wait, closed
};
int logfd = -1;

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
        char lenstr[10][2];

        sscanf(buf,"%s%s",lenstr[0],lenstr[1]);
        strcpy(buf,buf+strlen(lenstr[0])+strlen(lenstr[1]));

        sts->s_bullsize = atoi(lenstr[0]);
        sts->s_bullets = malloc(sizeof(struct pos)*sts->s_bullsize);
        sts->o_bullsize = atoi(lenstr[1]);
        sts->o_bullets = malloc(sizeof(struct pos)*sts->o_bullsize);
        str_to_data(buf, sts, (sts->s_bullsize),sts->o_bullsize);
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
    if(WRITE_LOG_TO_FILE) dprintf(logfd, "%c %d\n",ttybuf,(int)ttybuf);
    if(*phase == play){
        if (ttybuf == 'a' || ttybuf == 'A'){
            write(sock_fd, "OPT L", 5);
        } else if (ttybuf == 's' || ttybuf == 'S'){
            write(sock_fd, "OPT D", 5);
        } else if (ttybuf == 'd' || ttybuf == 'D'){
            write(sock_fd, "OPT R", 5);
        } else if (ttybuf == 'w' || ttybuf == 'W'){
            write(sock_fd, "OPT U", 5);
        } else if (ttybuf == ' '){
            write(sock_fd, "OPT S", 5);
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
    clear();
    endwin();			/* reset the tty etc	游戏退出时调用*/
    pause();
    exit(flg);
}

void handle_msg(struct status *sts, int sock_fd, int epollfd, enum client_phase *phase){
    char buf[1024];
    int nread;
    char cmd[5],content[1024];
    memset(buf, 0, sizeof(buf));

    if(-1==(nread = read(sock_fd,buf,sizeof(buf)))){
        perror("read");
        std_exit(EXIT_FAILURE,sock_fd,epollfd);
    }
    if(nread == 0){
        fprintf(stderr,"server is out of connection\n");
        std_exit(EXIT_FAILURE,sock_fd,epollfd);
    }
    sscanf(buf,"%s%s",cmd,content);
    strcpy(content,buf+strlen(cmd)+1);

    if(strcmp(cmd, "WAIT") == 0){
        *phase = conn_wait;
        if(WRITE_LOG_TO_FILE) dprintf(logfd, "wait %s %s\n",cmd, content);
    }else if(strcmp(cmd, "EXIT") == 0){
        *phase = closed;
        if(WRITE_LOG_TO_FILE) dprintf(logfd, "exit %s %s\n",cmd, content);
        
        show_end(strcmp(content, "3")==0?"You win!":"You're dead!");
    }else if(strcmp(cmd,"POST") == 0){
        int size;
        char tok[100], o_tok[100];

        if(*phase == conn_wait) 
            *phase = play;
        if(WRITE_LOG_TO_FILE) dprintf(logfd, "post %s %s\n",cmd, content);
        sscanf(content, "%s %s", tok, o_tok);
        str_to_data(content+strlen(tok)+strlen(o_tok)+2, sts, atoi(tok), atoi(o_tok));
    }
}

void print_sts(struct status *sts){
    printf("pos: self:%d %d other:%d %d\n", sts->self.c, sts->self.l,sts->other.c, sts->other.l);
}

int main(int argc, char **argv){
    int sock_fd;
    struct status sts;
    int readn;
    int epollfd;
    struct epoll_event events[2];
    struct termios tty;
    enum client_phase phase = init;

    initscr();			    /* turn on curses	*/
    clear();			    /* draw some stuff	*/
    
    if(argc < 3){
        fprintf(stderr, "usage: not enough args. Needed: ip, portnumber");
        exit(EXIT_FAILURE);
    }

    if(WRITE_LOG_TO_FILE){
        time_t tt;    //typedef long time_t;   
        struct tm *t;
        char log_filename[100];
        time(&tt);    //获取系统时间
        t = localtime(&tt);
        sprintf(log_filename, "%02d-%02d-%02d-%02d.log",\
            t->tm_hour, t->tm_min, t->tm_sec, rand());
        logfd = open(log_filename, O_WRONLY|O_CREAT, S_IRWXU);
    }
    init_client_socket(&sock_fd, argv[1], argv[2]);
    init_epoll(&epollfd, sock_fd);

    while(phase != closed){
        int nfds = epoll_wait(epollfd, events,2,-1);
        int i;

        if(nfds == -1){
            if(errno == EINTR) continue;
            perror("epoll_wait");
            std_exit(EXIT_FAILURE,sock_fd,epollfd);
        }
        for(i=0;i<nfds;i++){
            int fd = events[i].data.fd;
            if(fd == 0){
                //tty 
                send_opt(fd, sock_fd, &phase);
            }else {
                handle_msg(&sts,sock_fd,epollfd, &phase);
            }
        }
        if(phase == play){
            // print_sts(&sts);
            draw_status(&sts, NULL, logfd);
        }
    }

    getch();			/* wait for user input	*/
	endwin();			/* reset the tty etc	游戏退出时调用*/

    std_exit(EXIT_SUCCESS,sock_fd,epollfd);
}