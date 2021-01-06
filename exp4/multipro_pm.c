#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>          // shmat
#include <sys/shm.h>            // shmat shmget
#include <sys/ipc.h>            // shmget
#include <fcntl.h>              /* For O_* constants */
#include <sys/stat.h>           /* For mode constants */
#include <semaphore.h>          // sem_open
#include <dirent.h>             // PATH_MAX
#include <unistd.h>
#include <sys/wait.h>

#define MAX_ENTRY 1000  
#define MAX_ENTRY_LEN 100
#define MAX_PROCESS 10

#define DATA_KEY 99
#define CNTR_KEY 88

int get_files(const char *, const char *, char **, int*);
int isdirempty(const char *);
int match(const char *str, const char *pattern);
int kmp(const char * S,const char * T);
void get_next_arr(const char*T,int *next);

int main(int ac, char *av[]){
    char **jobs;
    int j_ptr = 0;
    int *cntr;
    int seg_id;
    sem_t *mutex;
    char suffix[1024];
    char curpath[1024];
    int i;
    int fkrv = -1, mcrv;
    char *fetched;

    if(ac < 2){
        fprintf(stderr, "usage: pattern needed.\n");
        exit(EXIT_FAILURE);
    }

    jobs = malloc(sizeof(char)*MAX_ENTRY);
    printf("%d files in total\n", j_ptr);
    
    get_files("/usr/include", "/usr/include", jobs, &j_ptr);

    printf("%d files in total\n", j_ptr);

    seg_id = shmget(CNTR_KEY, sizeof(int), IPC_CREAT|0777);
    cntr = shmat(seg_id, NULL, 0);
    *cntr = 0;

    if((mutex = sem_open("mutexsem", O_CREAT, 6440, 1)) == SEM_FAILED) {
        perror("sem_open");
        sem_unlink("mutexsem");
        exit(EXIT_FAILURE);
    }

    for(i=0;i<MAX_PROCESS && (fkrv = fork()) != 0;++i);

    sem_wait(mutex);
    while((*cntr) < j_ptr){
        fetched = jobs[(*cntr)++];
        sem_post(mutex);
        if(1 == (mcrv=match(fetched, av[1]))){
            printf("%s\n", fetched);
        }
        sem_wait(mutex);
    }
    sem_post(mutex);

    sem_unlink("mutexsem");

    return 0;
}
int match(const char *str, const char *pattern){
    return kmp(str, pattern)==-1?0:1;
}
int get_files(const char *suffix, const char *path, char **jobs, int *j_ptr){
    DIR* dirp;
    struct dirent *direntp;
    struct stat statbuf;
    char *nextsuff;
    char *nextpath;

    if(NULL == (dirp = opendir(path))){
        perror("get_files opendir");
        exit(1);
    }
    // printf("%s:\n",suffix);
    if(0 != isdirempty(path)){
        while(NULL != (direntp = readdir(dirp))){
            if((direntp -> d_name) != "." && (direntp -> d_name) != ".."){
                // printf("%s ",(direntp->d_name));
                jobs[(*j_ptr)] = malloc(MAX_ENTRY_LEN);
                strcpy(jobs[(*j_ptr)], suffix);
                strcat(jobs[(*j_ptr)], "/");
                strcat(jobs[(*j_ptr)++], direntp->d_name);
            }
        }
        // printf("\n\n");
    }
    dirp = opendir(path);
    while(NULL != (direntp = readdir(dirp))){
        if((direntp -> d_name)[0] != '.'){
            nextpath = strcpy(malloc(PATH_MAX),path);
            nextsuff = strcpy(malloc(PATH_MAX),suffix);
            strcat(strcat(nextpath, "/"), (direntp->d_name));
            strcat(strcat(nextsuff, "/"), (direntp->d_name));
            if(-1 == lstat(nextpath, &statbuf)){
                perror("get_files lstat");
                exit(1);
            }
            if(S_ISDIR(statbuf.st_mode)){
                get_files(nextsuff,nextpath, jobs,j_ptr);
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
void get_next_arr(const char*T,int *next){
    int i=1;
    next[1]=0;
    int j=0;
    while (i<strlen(T)) {
        if (j==0||T[i-1]==T[j-1]) {
            i++;
            j++;
            next[i]=j;
        }else{
            j=next[j];
        }
    }
}
int kmp(const char * S,const char * T){
    int next[10];
    get_next_arr(T,next);//根据模式串T,初始化next数组
    int i=1;
    int j=1;
    while (i<=strlen(S)&&j<=strlen(T)) {
        //j==0:代表模式串的第一个字符就和当前测试的字符不相等；S[i-1]==T[j-1],如果对应位置字符相等，两种情况下，指向当前测试的两个指针下标i和j都向后移
        if (j==0 || S[i-1]==T[j-1]) {
            i++;
            j++;
        }
        else{
            j=next[j];//如果测试的两个字符不相等，i不动，j变为当前测试字符串的next值
        }
    }
    if (j>strlen(T)) {//如果条件为真，说明匹配成功
        return i-(int)strlen(T);
    }
    return -1;
}