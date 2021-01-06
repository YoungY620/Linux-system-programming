/*
    就差做动画了
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
	EXIT [0/1/2/3/4]	确认退出	[主动/被动/拒绝访问/获胜/失败]
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
#include "game.h"

#define MAX_EVENTS 10

enum server_phase {
	init, conn_wait, serve, closed
};
int winr = -1;
struct status sts[2];
int player_fds[2];
enum server_phase phase = init;

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

void post_status(struct status *stss, int *playerfds){
	int i;
	for(i=0;i<2;i++){
		char msg[128] = "POST ";
		data_to_str(&stss[i],msg+strlen(msg));
		// sprintf(msg,"POST %s", msg);
		write(playerfds[i], msg, strlen(msg));
	}
}

void init_status_world(struct status *stss, int *playerfds){ // 改成init_world
	int i;
	for (i=0;i<2;i++){
		init_status(&stss[i]);
	}
	post_status(stss, playerfds);
}

void update_status(struct status *stss, int *playerfds, int pidx, char op,\
					enum server_phase *phase, struct epoll_event *events){
	if(op == 'Q'){
		*phase = closed;
		int j;
		for(j=0;j<2;j++){
			char quit_msg[100];
			strcpy(quit_msg, (j == pidx)?"EXIT 0\0":"EXIT 1\0");
			write(events[j].data.fd, quit_msg, strlen(quit_msg));
			close(events[j].data.fd);
		}
	} else {
		int res_code = -1;
		char dstr[1024];
		memset(dstr, 0, sizeof(dstr));

		opt_step(&stss[pidx],op,&stss[(int)(0==pidx)], &res_code);
		winr = (res_code == 0?pidx:((int)(0==pidx)));
		post_status(stss, playerfds);

		data_to_str(&stss[0], dstr);
		printf("0: %s\n",dstr);
		data_to_str(&stss[1], dstr);
		printf("1: %s\n",dstr);
	}
}

void handle_msg(struct epoll_event *events, struct status *stss, int i, enum server_phase *phase){
	char buf[1024];
	// handle options and update the status;
	int playerfd = events[i].data.fd;
	int nread;
	memset(buf,0,sizeof(buf)/sizeof(char));

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
			printf("in opt %c\n", content[0]);
			int pidx = (int)(playerfd == player_fds[1]);
			update_status(stss, player_fds, pidx, content[0], phase, events);
		}
	}
}

void move_bullets(int signum){
	periodic_step(&sts[0],&sts[1],&winr);
	post_status(sts, player_fds);
	// 如果产生胜者，终止计时器，phase置为closed，向所有客户端发送exit报文
	if(winr != -1){
		int i;

		struct itimerval itval;
		itval.it_value.tv_sec = 0;
		itval.it_value.tv_usec = 0;
		itval.it_interval.tv_sec = 0;
		itval.it_interval.tv_usec = 0;
		if(-1 == setitimer(ITIMER_REAL, &itval, NULL)){
			perror("setitimer");
			exit(EXIT_FAILURE);
		}
		phase = closed;

		for(i=0;i<2;i++){
			char end_msg[10] = "EXIT ";
			strcat(end_msg, winr == i?"3\0":"4\0");
			write(player_fds[i],end_msg,strlen(end_msg));
			close(player_fds[i]);
		}
	}
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

	init_socket(&listen_sock,av[1]);
	init_epoll(&epollfd,listen_sock);
	wait_players(epollfd, events,listen_sock,player_fds,&phase);
	init_status_world(sts, player_fds);
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
				handle_msg(events, sts ,i,&phase);
			}
		}
	}
    return 0;
}
