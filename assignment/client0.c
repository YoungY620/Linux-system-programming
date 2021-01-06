/*
    implement receive(&send) and decode a frame from server
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

#include "data.h"

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

void receive(char *buf, struct status *sts){
        char lenstr[10];

        sscanf(buf,"%s",lenstr);
        strcpy(buf,buf+strlen(lenstr));

        sts->bull_size = atoi(lenstr);
        sts->bullets = malloc(sizeof(struct pos)*sts->bull_size);
        str_to_data(buf, sts, (sts->bull_size));
}

int main(int argc, char **argv){
    int sock_fd;
    struct sockaddr_in addr;
    struct status sts;
    char buf[1024];
    int readn;

    if(argc < 3){
        fprintf(stderr, "usage: not enough args. Needed: ip, portnumber");
        exit(1);
    }
    
    init_client_socket(&sock_fd, argv[1], argv[2]);

    write(sock_fd, "GET /server.c", strlen("GET /server.c"));

    while(0!=(readn = read(sock_fd, buf, sizeof(buf)))){
        if(readn == -1){
            perror("read");
            exit(1);
        }

        receive(buf,&sts);

        // show
        // printf("str to data: %s\n",buf);
        // printf("%d %d\n",sts.self.c,sts.self.l);
        // printf("%d %d\n",sts.other.c,sts.other.l);
        // int i;
        // for(i=0;i<sts.bull_size;i++){
        //     printf("%d, %d\n",sts.bullets[i].c,sts.bullets[i].l);
        // }
    }
    return 0;
}