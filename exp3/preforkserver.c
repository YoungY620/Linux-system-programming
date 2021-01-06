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

#define N_PRO 32

void handle_get(int fd, char path[]){
    char buf[1024];
    int n;
    int filefd;

    if((filefd=open(path+1, O_RDONLY))==-1){
        char *notfound="HTTP/1.0 404 Not Found\n\
Content-Type: text/html; charset=UTF-8\r\n\r\n\
<html>\n\
<head><title>400 Bad Request</title></head>\n\
<body>\n\
<center><h1>400 Bad Request</h1></center>\n\
<hr><center>俺也不知道该说啥，反正是没找到</center>\n\
</body>\n\
</html>\n";
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
int main(int ac, char *av[]){
    int tcp_socket;
    struct sockaddr_in addr;
    int fd;
    char buf[1024];
    int n;
    char cmd[512];
    char path[512];
    int i;
    int rv_fork;

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

    for(i=0;i<N_PRO && 0!=(rv_fork=fork());i++);

    while(1){
        fd = accept(tcp_socket, NULL, NULL);
        n = read(fd, buf, sizeof(buf));
        printf("############# read #############\n");
        printf("%s\n",buf);
        sscanf(buf, "%s%s", cmd, path);
        if(strcmp(cmd, "GET")==0){
            handle_get(fd, path);
            close(fd);
        }else {
            char *badrequest="HTTP/1.0 400 Bad request\r\n\r\n<html><head>\n<title>400 Bad Request</title>\n</head><body>\n<h1>Bad Request</h1>\n</body></html>\n";
            write(fd, badrequest, strlen(badrequest));
            close(fd);
        }
    }
    return 0;
}