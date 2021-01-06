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

int main(int ac, char *av[]){
	struct sockaddr_in addr;
	char buf[1024];
	int n;
	char cmd[512];
	char path[512];

	int sockfd;
	struct epoll_event ev, events[MAX_EVENTS];
	int listen_sock, conn_sock, nfds, epollfd;

	signal(SIGPIPE, SIG_IGN);

	listen_sock  =  socket(AF_INET,  SOCK_STREAM, 0);

	addr.sin_family = AF_INET;
	addr.sin_port = htons(atoi(av[1]));
	addr.sin_addr.s_addr = INADDR_ANY;

	if(bind(listen_sock, (const struct sockaddr *)&addr, sizeof(struct sockaddr_in))==-1){
		perror("cannot bind");
		exit(1);
	}	

	listen(listen_sock, 1);

	epollfd = epoll_create(10);
	if (epollfd == -1) {
		perror("epoll_create");
		exit(EXIT_FAILURE);
	}

	ev.events = EPOLLIN;
	ev.data.fd = listen_sock;
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listen_sock, &ev) == -1) {
		perror("epoll_ctl: listen_sock");
		exit(EXIT_FAILURE);
	}

	while(1){
		nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
		// printf("wait\n");
		if (nfds == -1) {
			perror("epoll_pwait");
			exit(EXIT_FAILURE);
		}

		for (n = 0; n < nfds; ++n) {
			if (events[n].data.fd == listen_sock) {
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
				sockfd=events[n].data.fd;
				nread = read(sockfd, buf, sizeof(buf));

				if(nread == -1 && errno == EBADF){
					printf("[%d]bad fd\n",n);
					// continue;
				}
				if ( nread == 0) {
					/*4connection closed by client */
					close(sockfd);
				} else {
					sscanf(buf, "%s%s", cmd, path);
					if(strcmp(cmd, "GET")==0){
						handle_get(sockfd, path);
					}
					close(sockfd);  /*close the sockfd*/
				}
			}
		}
	}
    return 0;
}
