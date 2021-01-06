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
#define TIMEOUT 500
#define PATH_STK_SIZE (PATH_MAX/2+1)
#define BUF_SIZE 1024

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
        // printf("waiting...\n");
        n_events = epoll_wait(epfd, events, N_EVENTS, TIMEOUT);

        for(i=0;i<n_events;++i){
            printf("fd = %d, events:%d,%d,%d\n",events[i].data.fd,events[i].events,EPOLLIN,EPOLLOUT);
        }

        for(i=0;i<n_events;++i){
            if(events[i].data.fd == listenfd){
                if((connfd = accept(listenfd, NULL, NULL)) < 0){
                    perror("accept");
                    exit(1);
                }
                printf("new connection fd: %d\n",connfd);
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

                while(1){
                    char buf[BUF_SIZE];
                    char cmd[BUF_SIZE/2];
                    char path[BUF_SIZE/2];
                    char err_msg[BUF_SIZE];
                    n = read(fd, buf, sizeof(buf));
                    if(n == -1 || n == 0) break;
                    printf("############# read #############\n");
                    printf("%s %d\n",buf,n);

                    sscanf(buf, "%s%s", cmd, path);
                    if(-1==handle_url(path, err_msg)){
                        fprintf(stderr,"%s\n",err_msg);
                        close(fd);
                        break;
                    }
                    if(strcmp(cmd, "GET")==0){
                        handle_get(fd, path);
                        close(fd);
                    }else {
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
    printf("############# write ###############\n");
    write(fd, "HTTP/1.0 200 OK\r\n\r\n", 19);
    printf("HTTP/1.0 200 OK\r\n\r\n");
    while((n=read(filefd, buf, sizeof(buf)))>0){
        write(fd, buf, n);
        printf("%s",buf);
    }
    printf("\n");
    close(filefd);
}

int handle_url(char url[], char *err_msg){
    //todo 处理url攻击：语义url攻击
    char *dir_stack[PATH_STK_SIZE];
    int ptr = 0;
    char cp[PATH_MAX], *dir;
    char ans[PATH_MAX] = "";
    int i;

    strcpy(cp,url);
    
    for(dir = strtok(cp,"/");dir!=NULL;dir = strtok(NULL,"/")){
        if(strcmp(dir, ".") == 0){
            continue;
        }
        if(strcmp(dir, "..") == 0){
            if(ptr == 0){
                sprintf(err_msg, "handle_url:invalid url,Found:%s",url);
                return -1;
            }
            --ptr;
        }
        printf("ptr:%d, url:\"%s\", url len:%ld\n",ptr,url,strlen(url));
        if(ptr == PATH_STK_SIZE){
            sprintf(err_msg,"handle_url:too long url. url length:%ld",strlen(url));
            return -1;
        }
        dir_stack[ptr++] = dir;
    }
    for (i=0;i<ptr;++i){
        strcat(strcat(ans,"/"),dir_stack[i]);
    }
    url[0]='\0';
    strcat(url,ans);
    return 0;
}