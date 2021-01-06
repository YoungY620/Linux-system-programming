#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>          /* See NOTES */
#include<sys/socket.h>
#include<arpa/inet.h>
#include<string.h>

int main(int ac, char *av[]){
    int sock_fd;
    struct sockaddr_in addr;
    int fd;
    char buf[1024];

    if(1 == ac){
        perror("usage: portnumber needed");
        exit(1);
    }

    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(sock_fd == -1){
        perror("socket");
        exit(1);
    }

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(atoi(av[1]));

    if(-1 == bind(sock_fd, (const struct sockaddr *)&addr, sizeof(struct sockaddr_in))){
        perror("bind");
        exit(1);
    }

    listen(sock_fd, 1);

    while(1){
        int n;

        fd = accept(sock_fd, NULL, NULL);

        while(0!=(n=read(fd, buf, sizeof(buf)))){
            write(fd, buf, n);
        }

        close(fd);
    }

    return 0;
}