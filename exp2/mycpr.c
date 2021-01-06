#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

int traverse(const char *from, const char *to);
int copy(const char *pathname, const char *to);
int copy_file(const char *pathname, const char *to);

int main(int argc, char *argv[]){
	DIR *dirp;
	char *from_path;
	char *to_path;
	char *curpath;
	int shortname_start;
	
	if (3 != argc){
		printf("NOT ENOUGH ARGS!\n");
		exit(1);
	}
	from_path=argv[1];
	to_path=argv[2];
	
	// printf("from_path: %s\n",from_path);
	// printf("to_path: %s\n",to_path);
	// printf("%s\n",(curpath=getcwd(malloc(PATH_MAX),PATH_MAX)));

	curpath=getcwd(malloc(PATH_MAX),PATH_MAX);
	curpath=strcat(curpath,"/");
	if(from_path[0]!='/')
		from_path = strcat(strcpy(malloc(PATH_MAX),curpath),from_path);
	if(to_path[0]!='/')
		to_path = strcat(strcpy(malloc(PATH_MAX),curpath),to_path);	

	if(opendir(to_path)!=NULL){
		shortname_start = strlen(from_path);
		while((shortname_start--) && from_path[shortname_start]!='/');
		strcat(strcat(to_path,"/"), (char *)(from_path+shortname_start));
	}

	if(copy(from_path,to_path) != 0){
		exit(1);
	}
	return 0;
}

int traverse(const char *path, const char *to){
	DIR *dirp;
	struct dirent *direntp;
	char *tpath;
	char *tto;
	
	if(NULL==(dirp=opendir(path))){
		perror(strcat("CANNOT OPEN \n",path));
		return -1;
	}
	while(NULL != (direntp=readdir(dirp))){
		if(strcmp(direntp -> d_name, ".")!=0 
		&& strcmp(direntp -> d_name, "..")!=0){
			// printf("traverse: %s\n",direntp->d_name);
			tpath = strcpy(malloc(PATH_MAX),path);
			tto = strcpy(malloc(PATH_MAX),to);
			strcat(strcat(tpath,"/"), direntp->d_name);
			strcat(strcat(tto,"/"), direntp->d_name);
			if(0 != copy(tpath, tto)){
				return -1;
			}
		}
	}
	if(0!=closedir(dirp)){
		perror("CLOSE DIR ERROR");
		return -1;
	}
	return 0;
}

int copy(const char *pathname, const char *to){
	struct stat statbuf;
	// printf("copy: %s %s\n",pathname,to);
	if(-1 == stat(pathname, &statbuf)){
		perror("stat()");
		return -1;
	}
	if(S_ISDIR(statbuf.st_mode)){
		// TODO handle when file exist!
		if (-1==mkdir(to, S_IRWXU)){
			perror("CANNOT CREATE DIR");
			return -1;
		}
		traverse(pathname, to);
	} else {
		copy_file(pathname,to);
	}
	return 0;
}

int copy_file(const char *from_path, const char *to_path) {
	int from;
	int to;
	int bsize = 1024;
	char buff[1024];
	int count;
	if((from=open(from_path, O_RDONLY))==-1){
			perror("CANNOT OPEN!");
			return -1;
	}
	if((to=open(to_path, O_WRONLY|O_CREAT, S_IRWXU))==-1){
			perror("CANNOT OPEN OR CREATE");
			return -1;
	}
	while(0 != (count=read(from, buff, bsize))){
			if (count<0){
					perror("READ ERROR");
					return -1;
			}
			if(write(to,buff,count)!=count){
					perror("WRITE ERROR");
					return -1;
			}
	}
	close(from);
	close(to);
	return 0;
}
