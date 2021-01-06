/*
    实现了数据的传递、通信的各个阶段，只差游戏逻辑和图形界面
 */
/*
	client:
	1. （如果有）发送opt
	2. 接收重绘status

	server：
	1. 发送status （周期（即子弹移动的周期）或有OPT发来）
	3. 接收opt
*/
/*
	协议：
	POST [bull_size self_pos other_pos bullet_poses] 
	OPT [U/D/L/R]	上下左右
	EXIT [0/1/2]	确认退出	[主动/被动/拒绝访问]
	WAIT			等待其他player
*/
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/epoll.h>
#include <string.h>
#include <errno.h>
#include <signal.h>			// signal
#include <sys/time.h>
// #include <curses.h>

#include "data.h"

#define MAX_EVENTS 10
#define COLS 120
#define LINES 30

enum server_phase {
	init, conn_wait, serve, closed
};
struct status sts[2];
int player_fds[2];

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

void init_socket(int *listen_sock, char port[]){
	struct sockaddr_in addr;

	*listen_sock  =  socket(AF_INET,  SOCK_STREAM, 0);

	addr.sin_family = AF_INET;
	addr.sin_port = htons(atoi(port));
	addr.sin_addr.s_addr = INADDR_ANY;
	if(bind(*listen_sock, (const struct sockaddr *)&addr, sizeof(struct sockaddr_in))==-1){
		perror("cannot bind");
		exit(1);
	}	

	listen(*listen_sock, 1);
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
}

void wait_players(int epollfd, struct epoll_event *events,\
				int listen_sock, int *conn_sock, enum server_phase *phase){
	int nfds;
	struct epoll_event ev;
	int i;

	for(i=0;i<2;i++){
		char wating_msg[10] = "WAIT\0";
		nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
		
		if (nfds == -1) {
			perror("waiting_players epoll_pwait");
			exit(EXIT_FAILURE);
		}
		conn_sock[i] = accept(listen_sock, NULL, NULL);
		if(conn_sock[i] == -1 && errno == EAGAIN){
			// 另一个客户端未连接，第一个客户端已断开：close并重连（i--）
			i = -1;
			*phase = init;
			close(conn_sock[0]);
			continue;
		}else if(conn_sock[i] == -1){
			perror("accept");
			printf("%d\n",errno);
			exit(EXIT_FAILURE);
		}
		setnonblocking(conn_sock[i]);
		ev.events = EPOLLIN | EPOLLET;
		ev.data.fd = conn_sock[i];
		if (epoll_ctl(epollfd, EPOLL_CTL_ADD, conn_sock[i], &ev) == -1) {
			perror("epoll_ctl: conn_sock");
			exit(EXIT_FAILURE);
		}
		if(-1==write(conn_sock[i], wating_msg, strlen(wating_msg))){
			perror("write");
			exit(EXIT_FAILURE);
		}
		printf("connected %d\n",conn_sock[i]);
		*phase = conn_wait;
	}
}

void init_status(struct status *stss, int *playerfds){
	int i;
	for (i=0;i<2;i++){
		char msg[128]="POST ";
		char content[128];
		
		stss[i].bull_size = 0;
		stss[i].self.c = 0;
		stss[i].self.l = LINES-3;
		stss[i].other.c = COLS-6;
		stss[i].other.l = 0;
		stss[i].bullets = malloc(100*sizeof(struct pos)); // todo to be freed

		data_to_str(&stss[i], content);
		strcat(msg,content);
		printf("%s\n",msg);
		write(playerfds[i], msg, strlen(msg));
	}
}

void post_status(struct status *stss, int *playerfds){
	int i;
	for(i=0;i<2;i++){
		char msg[128] = "POST ";
		data_to_str(&stss[i],msg+strlen(msg));
		// sprintf(msg,"POST %s", msg);
		write(playerfds[i], msg, strlen(msg));
	}
}

void update_status(struct status *stss, int *playerfds, int pidx, char op){
	if(op == 'Q'){

	}else{
		// todo update logic
		post_status(stss, playerfds);
	}
}

void handle_msg(struct epoll_event *events, int i, enum server_phase *phase){
	char buf[1024];
	// handle options and update the status;
	int playerfd = events[i].data.fd;
	int nread;
	memset(buf,sizeof(buf)/sizeof(char),' ');

	if((nread = read(playerfd, buf, sizeof(buf))) == -1){
		perror("read");
		exit(EXIT_FAILURE);
	}
	printf("%d %s\n",playerfd,buf);
	if(nread == 0){
		// closed by clients
		*phase = closed;
	}else{
		char cmd[128], content[128];
		sscanf(buf,"%s%s",cmd,content);
		strcpy(content, buf+strlen(cmd)+1);
		printf("%s %s\n",cmd,content);

		if(strcmp(cmd,"OPT") == 0){
			int pidx = (int)(playerfd == player_fds[1]);
			if(strcmp(content,"Q")==0){
				*phase = closed;
				int j;
				for(j=0;j<2;j++){
					char quit_msg[100];
					strcpy(quit_msg, (j == pidx)?"EXIT 0\0":"EXIT 1\0");
					write(events[j].data.fd, quit_msg, strlen(quit_msg));
					close(events[j].data.fd);
				}
			} else {
				// handle option update status;	
				update_status(sts, player_fds, pidx, content[0]);
			}
		}
	}
}

void move_bullets(int signum){
	//logic
	printf("move bullets\n");
	post_status(sts, player_fds);
}

void init_mv_bullets_timer(){
	struct itimerval itval;
	if(SIG_ERR==signal(SIGALRM, move_bullets)){
		perror("signal");
		exit(EXIT_FAILURE);
	}
	itval.it_value.tv_sec = 1;
	itval.it_value.tv_usec = 0;
	itval.it_interval.tv_sec = 1;
	itval.it_interval.tv_usec = 0;
	if(-1 == setitimer(ITIMER_REAL, &itval, NULL)){
		perror("setitimer");
		exit(EXIT_FAILURE);
	}
}

int main(int ac, char *av[]){
	int sockfd;
	struct epoll_event events[MAX_EVENTS];
	int listen_sock, nfds, epollfd;
	enum server_phase phase = init;

	init_socket(&listen_sock,av[1]);
	init_epoll(&epollfd,listen_sock);
	wait_players(epollfd, events,listen_sock,player_fds,&phase);
	init_status(sts, player_fds);
	init_mv_bullets_timer();

	while(phase != closed){
		int i;
		nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
		
		if (nfds == -1 && errno != EINTR) {
			perror("epoll_pwait");
			exit(EXIT_FAILURE);
		}
		for (i = 0; i < nfds && phase != closed; ++i) {
			if (events[i].data.fd == listen_sock) {
				int conn_sock = accept(listen_sock, NULL, NULL);
				char refuse_msg[10] = "EXIT 2\0";
				if(-1==write(conn_sock, refuse_msg, strlen(refuse_msg))){
					perror("write");
					exit(EXIT_FAILURE);
				}
			} else {
				handle_msg(events,i,&phase);
			}
		}
	}
    return 0;
}
