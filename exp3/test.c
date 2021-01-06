#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h> // PATH_MAX
#include <string.h>

int main(){
    // char buf[PATH_MAX];
    // printf("%s\n",getcwd(buf,PATH_MAX));
    // printf("%s\n",getenv("PWD"));
    // chdir("/usr");
    // printf("%s\n",getcwd(buf,PATH_MAX));
    // printf("%s\n",getenv("PWD"));

    // char a[100]="000\0", b[100]="123456789\0";
    // printf("%d\n",strncmp(b,a,strlen(a)));

    char a[100]="000\0", b[100]="123456789\0";
    strcpy(b,a);
    printf("%s\n",b);
    return 0;
}