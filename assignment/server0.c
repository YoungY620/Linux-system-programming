/*
    implement receive&send and decode&encode a frame from server to client
 */

// use fd
/*
	client:
	1. （如果有）发送opt
	2. 接收重绘status

	server：
	1. 发送status （周期（即子弹移动的周期）或有OPT发来）
	3. 接收opt
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

#include "data.h"

#define MAX_EVENTS 10

void handle_get(int fd, char path[]){
	char buf[1024];
	int n;
	int filefd;
	if((filefd=open(path+1, O_RDONLY))==-1){
		write(fd, "HTTP/1.0 404 Not Found\r\n\r\n", 26);
		return;
	}
	write(fd, "HTTP/1.0 200 OK\r\n\r\n", 19);
	while((n=read(filefd, buf, sizeof(buf)))>0){
		write(fd, buf, n);
	}	
	close(filefd);
}

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

int main(int ac, char *av[]){
	char buf[1024];
	int i;

	int sockfd;
	struct epoll_event ev, events[MAX_EVENTS]; // events只是容器
	int listen_sock, conn_sock, nfds, epollfd;

	init_socket(&listen_sock,av[1]);

	init_epoll(&epollfd,listen_sock);

	while(1){
		nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
		if (nfds == -1) {
			perror("epoll_pwait");
			exit(EXIT_FAILURE);
		}

		for (i = 0; i < nfds; ++i) {
			if (events[i].data.fd == listen_sock) {
				conn_sock = accept(listen_sock, NULL, NULL);
				if (conn_sock == -1) {
					perror("accept");
					exit(EXIT_FAILURE);
				}
				setnonblocking(conn_sock);
				ev.events = EPOLLIN | EPOLLET;
				ev.data.fd = conn_sock;
				if (epoll_ctl(epollfd, EPOLL_CTL_ADD, conn_sock,
							&ev) == -1) {
					perror("epoll_ctl: conn_sock");
					exit(EXIT_FAILURE);
				}
			} else {
				int nread;
				sockfd=events[i].data.fd;
                struct status ss;
                ss.self.c = 4;
                ss.other.c = 2;
                ss.self.l = 3;
                ss.other.l = 5;
                ss.bullets = malloc(sizeof(struct pos)*3);
				ss.bullets[0].c = 6;
				ss.bullets[0].l = 5;
				ss.bullets[1].c = 4;
				ss.bullets[1].l = 3;
				ss.bullets[2].c = 2;
				ss.bullets[2].l = 1; 
				ss.bull_size = 3;

				if ( (nread = read(sockfd, buf, sizeof(buf))) == 0) {
					/*4connection closed by client */
					close(sockfd);
				} else {
					char cmd[512];
					char path[512];

					sscanf(buf, "%s%s", cmd, path);
					if(strcmp(cmd, "GET")==0){
						char ssbuf[1024];
						printf("123\n");
						data_to_str(&ss, ssbuf);
						printf("data to str: %s\n",ssbuf);
                        write(sockfd, ssbuf, strlen(ssbuf));	// 一定要用报文的总长度，而不是buf的大小(sizeof(buf))!
						// handle_get(sockfd, path);
					}
					close(sockfd);  /*close the sockfd*/
				}
			}
		}
	}
    return 0;
}
