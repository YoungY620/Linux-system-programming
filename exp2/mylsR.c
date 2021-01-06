#include<stdio.h>
#include<stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include<string.h>

int do_ls(const char *, const char *);
int isdirempty(const char *);

int main(int argc, char *argv[]){
    char *suffix;
    char *curpath = getcwd(malloc(PATH_MAX),PATH_MAX);

    if(1 == argc){
        suffix = "./";
    }else{
        suffix = argv[1];
    }

    if(suffix[strlen(suffix)-1] == '/'){
        suffix = strncpy(malloc(PATH_MAX),suffix,strlen(suffix)-1);
    }

    if(suffix[0] != '/'){
        strcat(strcat(curpath,"/"),suffix);
    }else{
        curpath = suffix;
    }

    do_ls(suffix, curpath);

    return 0;
}

int do_ls(const char *suffix, const char *path){
    DIR* dirp;
    struct dirent *direntp;
    struct stat statbuf;
    char *nextsuff;
    char *nextpath;

    if(NULL == (dirp = opendir(path))){
        perror("do_ls opendir");
        exit(1);
    }
    printf("%s:\n",suffix);
    if(0 != isdirempty(path)){
        while(NULL != (direntp = readdir(dirp))){
            if((direntp -> d_name)[0] != '.'){
                printf("%s ",(direntp->d_name));
            }
        }
        printf("\n\n");
    }

    dirp = opendir(path);
    while(NULL != (direntp = readdir(dirp))){
        if((direntp -> d_name)[0] != '.'){
            nextpath = strcpy(malloc(PATH_MAX),path);
            nextsuff = strcpy(malloc(PATH_MAX),suffix);
            strcat(strcat(nextpath, "/"), (direntp->d_name));
            strcat(strcat(nextsuff, "/"), (direntp->d_name));
            if(-1 == lstat(nextpath, &statbuf)){
                perror("do_ls lstat");
                exit(1);
            }
            if(S_ISDIR(statbuf.st_mode)){
                do_ls(nextsuff,nextpath);
            }
            free(nextpath);
            free(nextsuff);
            nextpath = NULL;
            nextsuff = NULL;
        }
    }
}
int isdirempty(const char *dirname){
    /* 打开要进行匹配的文件目录 */
    DIR *dir = opendir(dirname);
    struct dirent *ent;
    if (dir == NULL){  
        perror("seekkey.c-98-opendir");
        return -1;
    }  
    while (1){  
        ent = readdir (dir);
        if (ent <= 0){  
            break;
        }  
        if ((strcmp(".", ent->d_name)==0) || (strcmp("..", ent->d_name)==0)){  
            continue;
        }  
        /*判断是否有目录和文件*/
        if ((ent->d_type == DT_DIR) || (ent->d_type == DT_REG)){  
            return -1;
        }  
    }  
    return 0;
}