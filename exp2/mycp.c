#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
int main(int argc,char *argv[]){
        int from;
        int to;
        int bsize = 1024;
        char buff[1024];
        int count;
        if(argc<3){
                printf("NOT ENOUGH ARGS!\n");
                exit(1);
        }
        if((from=open(argv[1], O_RDONLY))==-1){
                perror("CANNOT OPEN!");
                exit(1);
        }
        if((to=open(argv[2], O_WRONLY|O_CREAT, S_IRWXU))==-1){
                perror("CANNOT OPEN OR CREATE");
                exit(1);
        }
        while(0 != (count=read(from, buff, bsize))){
                // printf("%s", buff);
                if (count<0){
                        perror("READ ERROR");
                        exit(1);
                }
                if(write(to,buff,count)!=count){
                        perror("WRITE ERROR");
                        exit(1);
                }
        }
		close(from);
		close(to);
        return 0;
}
