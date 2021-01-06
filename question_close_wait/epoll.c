/*
 *  替换了handle_url的算法
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
#include <string.h>
#include <sys/epoll.h>
#include <dirent.h>

#define N_EVENTS 20
#define EPOLL_SIZE 256
#define TIMEOUT -1
#define PATH_STK_SIZE (PATH_MAX/2+1)
#define BUF_SIZE 1024

#define LOG 1

void setnonblocking(int sock);
void handle_get(int fd, char path[]);
int handle_url(char *url, char *err_msg);

int main(int ac, char *av[]){
    int listenfd;
    struct sockaddr_in addr;
    struct epoll_event ev,events[N_EVENTS];
    int epfd;
    int connfd;
    
    // arg chack
    if(1 == ac){
        perror("usage: portnumber needed");
        exit(1);
    }

    // init epoll
    epfd=epoll_create(EPOLL_SIZE);

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    setnonblocking(listenfd);
    //设置与要处理的事件相关的文件描述符
    ev.data.fd=listenfd;
    //设置要处理的事件类型
    ev.events=EPOLLIN|EPOLLET;
    //ev.events=EPOLLIN;
    //注册epoll事件
    epoll_ctl(epfd,EPOLL_CTL_ADD,listenfd,&ev);
    
    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(av[1]));
    addr.sin_addr.s_addr = INADDR_ANY;
    if(bind(listenfd, (const struct sockaddr *)&addr, sizeof(struct sockaddr_in))==-1){
        perror("cannot bind");
        exit(1);
    }
    
    listen(listenfd, 1);

    while(1){
        int n_events;
        int i;
        //if(LOG) printf("waiting...\n");
        n_events = epoll_wait(epfd, events, N_EVENTS, TIMEOUT);

        for(i=0;i<n_events && LOG;++i){
            printf("fd = %d, events:%d,%d,%d\n",events[i].data.fd,events[i].events,EPOLLIN,EPOLLOUT);
        }

        for(i=0;i<n_events;++i){
            if(events[i].data.fd == listenfd){
                if((connfd = accept(listenfd, NULL, NULL)) < 0){
                    perror("accept");
                    exit(1);
                }
                if(LOG){
                    printf("new connection fd: %d\n",connfd);
                }
                setnonblocking(connfd);
                //设置用于读操作的文件描述符
                ev.data.fd=connfd;
                //设置用于注测的读操作事件
                // edge-triggered
                ev.events=EPOLLIN|EPOLLET;
                //ev.events=EPOLLIN;
                //注册ev
                if(-1==epoll_ctl(epfd,EPOLL_CTL_ADD,connfd,&ev)){
                    perror("epoll_ctl");
                    exit(1);
                }
            }else{
                int n;
                int fd = events[i].data.fd;
                char buf[BUF_SIZE];
                char cmd[BUF_SIZE/2];
                char path[BUF_SIZE/2];
                char err_msg[BUF_SIZE];

                n = read(fd, buf, sizeof(buf));
                if(LOG){
                    printf("############# read #############\n");
                    printf("%s\n",buf);
                }

                if(n == 0){
                    close(fd);
                }else{
                    sscanf(buf, "%s%s", cmd, path);
                    if(-1==handle_url(path, err_msg)){
                        fprintf(stderr,"%s\n",err_msg);
                        close(fd);
                    } else if(strcmp(cmd, "GET")==0){
                        handle_get(fd, path);
                        close(fd);
                    } else {
                        char *badrequest="HTTP/1.0 400 Bad request\r\n\r\n<html><head>\n<title>400 Bad Request</title>\n</head><body>\n<h1>Bad Request</h1>\n</body></html>\n";
                        write(fd, badrequest, strlen(badrequest));
                        close(fd);
                    }
                }
                
            }
        }
    }
    close(epfd);
    return 0;
}

void handle_get(int fd, char path[]){
    char buf[BUF_SIZE];
    int n;
    int filefd;

    if((filefd=open(path+1, O_RDONLY))==-1){
        char *notfound="HTTP/1.0 404 Not Found\r\n\r\n<html>\n<head><title>404 Not Found</title></head>\n<body>\n<h1>404 Not Found</h1>\n</body>\n</html>\n";
        write(fd, notfound, strlen(notfound));
        return;
    }
    if(LOG){
        printf("############# write ###############\n");
        printf("HTTP/1.0 200 OK\r\n\r\n");
    }
    write(fd, "HTTP/1.0 200 OK\r\n\r\n", 19);
    while((n=read(filefd, buf, sizeof(buf)))>0){
        write(fd, buf, n);
        if(LOG) printf("%100s",buf);
    }
    if(LOG) printf("\n");
    close(filefd);
}

int handle_url(char url[], char *err_msg){
    //处理url攻击：语义url攻击
    // char buf[PATH_MAX];
    // char *serverpath = getcwd(buf,PATH_MAX);
    // char cp[PATH_MAX],filename[PATH_MAX];
    // int i;
    // strcpy(cp,url);
    // for(i=strlen(cp)-1;i>=0 && cp[i] != '/';--i);
    // strcpy(filename, cp+i+1);
    // cp[i] = '\0';
    // while(cp[0] == '/'){
    //     strcpy(cp, cp+1);
    // }
    // if(strlen(cp) == 0){
    //     strcpy(cp,".");
    // }
    // if(-1==chdir(cp)){
    //     sprintf(err_msg,"chdir: cannot open. Found:%s",cp);
    //     chdir(serverpath);
    //     return -1;
    // }
    // strcpy(url,getcwd(buf,PATH_MAX));
    // strcat(strcat(url,"/"),filename);
    // chdir(serverpath);
    // if(0!=strncmp(url,serverpath,strlen(serverpath))){
    //     sprintf(err_msg,"strncmp: visiting invalid path.");
    //     return -1;
    // }
    // strcpy(url,url+strlen(serverpath));
    return 0;
}
void setnonblocking(int sock)
{
    int opts;
    opts=fcntl(sock,F_GETFL);
    if(opts<0)
    {
        perror("fcntl(sock,GETFL)");
        exit(1);
    }
    opts = opts|O_NONBLOCK;
    if(fcntl(sock,F_SETFL,opts)<0)
    {
        perror("fcntl(sock,SETFL,opts)");
        exit(1);
    }
}