#include<stdio.h>
#include<time.h>
#include<sys/types.h>
#include<dirent.h>
#include<sys/stat.h>
#include<stdlib.h>
#include<string.h>

void do_ls(char[]);
void dostat(char *,char *);
void show_file_info(char *,struct stat *);
void mode_to_letters(int,char[]);
char * uid_to_name(uid_t);
char * gid_to_name(gid_t);

int main(int argc,char *argv[]){
        char dirname[PATH_MAX];

        if(argc==1)
                strcpy(dirname,".");
        else if(argc==2)
                strcpy(dirname,argv[1]);
        else{
                printf("NOT ENOUGH ARGS!\n");
                exit(1);
        }
        do_ls(dirname);
        return 0;
}

void do_ls(char* dirname){
        DIR *dir_ptr;
        struct dirent *direntp;

        // printf("dirname[(sizeof(dirname))-1] = %c\n",dirname[(strlen(dirname))-1]);
        if(dirname[strlen(dirname)-1]!='/'){
                // printf("123");
                strcat(dirname,"/");
        }
        // printf("dirname = %s\n",dirname);

        if((dir_ptr=opendir(dirname))==0){
                fprintf(stderr,"ls:cannot open %s\n",dirname);
        }else{
                while((direntp=readdir(dir_ptr))!=0)
                        if(direntp->d_name[0]!='.')
                                dostat(dirname,direntp->d_name);
                closedir(dir_ptr);
        }
}

void dostat(char *dir, char *file){
        struct stat info;
        char filename[PATH_MAX]  = "";
        strcat(strcat(filename,dir),file);
        if(lstat(filename,&info)==-1){
                printf("%s\n", filename);
                perror("lstat");
        } else
                show_file_info(file,&info);
}

void show_file_info(char *filename,struct stat *info_p){
        char modestr[11];
        mode_to_letters(info_p->st_mode,modestr);
        printf("%-12s",modestr);
        printf("%-4d",(int)info_p->st_nlink);
        printf("%-8s",uid_to_name(info_p->st_uid));
        printf("%-8s",gid_to_name(info_p->st_gid));
        printf("%-8ld",(long)info_p->st_size);
        time_t timelong=info_p->st_mtime;
        struct tm *htime=localtime(&timelong);
        printf("%02d月 %02d  %02d:%02d",htime->tm_mon+1,htime->tm_mday,htime->tm_hour,htime->tm_min);
        printf(" %s\n",filename);
}

void mode_to_letters(int mode,char str[]){
        strcpy(str,"----------");
        if(S_ISDIR(mode))   str[0]='d';
        if(S_ISCHR(mode))   str[0]='c';
        if(S_ISBLK(mode))   str[0]='b';

        if(mode & S_IRUSR)  str[1]='r';
        if(mode & S_IWUSR)  str[2]='w';
        if(mode & S_IXUSR)  str[3]='x';

        if(mode & S_IRGRP)  str[4]='r';
        if(mode & S_IWGRP)  str[5]='w';
        if(mode & S_IXGRP)  str[6]='x';

        if(mode & S_IROTH)  str[7]='r';
        if(mode & S_IWOTH)  str[8]='w';
        if(mode & S_IXOTH)  str[9]='x';
}

#include<pwd.h>
char * uid_to_name(uid_t uid){
        struct passwd *pw_str;
        static char numstr[10];
        if((pw_str=getpwuid(uid))==NULL){
                sprintf(numstr,"%d",uid);
                return numstr;
        }
        else
                return pw_str->pw_name;
}

#include<grp.h>
char * gid_to_name(gid_t gid){
        struct group *grp_ptr;
        static char numstr[10];
        if((grp_ptr=getgrgid(gid))==NULL){
                sprintf(numstr,"%d",gid);
                return numstr;
        }
        else
                return grp_ptr->gr_name;
}
