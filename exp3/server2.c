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
int main(int ac, char *av[]){
    int tcp_socket;
    struct sockaddr_in addr;
    int fd;
    char buf[1024];
    int n;
    char cmd[512];
    char path[512];

    if(1 == ac){
        perror("usage: portnumber needed");
        exit(1);
    }

    tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
    
    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(av[1]));
    addr.sin_addr.s_addr = INADDR_ANY;

    if(bind(tcp_socket, (const struct sockaddr *)&addr, sizeof(struct sockaddr_in))==-1){
        perror("cannot bind");
        exit(1);
    }
    
    listen(tcp_socket, 1);
    while(1){    
        fd = accept(tcp_socket, NULL, NULL);
        n = read(fd, buf, sizeof(buf));
        printf("#############\n");
        printf("%s\n",buf);

        sscanf(buf, "%s%s", cmd, path);
        if(strcmp(cmd, "GET")==0){
            handle_get(fd, path);
        }
        close(fd);
    }
    return 0;
}