#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
main()
{
    fd_set rfds;
    struct timeval tv;
    int retval;
    int mice_fd;
    char buf[1024];
    int n;
    // mice_fd=open("/dev/input/mice", O_RDONLY);
    if(mice_fd == -1){
        perror("mice fd");
        exit(EXIT_FAILURE);
    }
    while(1){
    FD_ZERO(&rfds);
    FD_SET(0, &rfds);
    // FD_SET(mice_fd, &rfds);
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    retval = select(mice_fd+1, &rfds, NULL, NULL, &tv);
    if (retval == -1){
        fprintf(stderr,"123\n");
        perror("select()");
        exit(1);
    }else if (retval){
        if(FD_ISSET(0, &rfds)){
            n=read(0, buf, sizeof(buf));
            printf("tty:%.*s\n", n, buf);
        }
        if(FD_ISSET(mice_fd, &rfds)){
            n=read(mice_fd, buf, sizeof(buf));
            printf("mice:%.*s\n", n, buf);
        }
    }else
        printf("No data within five seconds.\n");
    }
    exit(EXIT_SUCCESS);
}