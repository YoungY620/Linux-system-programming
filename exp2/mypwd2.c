#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h> // PATH_MAX

// TODO 多文件系统目录树？
int main(int argc, char *argv[]){
    char *pwd;
    pwd=getenv("PWD");
    printf("%s\n",pwd);
    return 0;
}