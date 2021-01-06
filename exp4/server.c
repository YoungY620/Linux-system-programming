#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <errno.h>

#define MSGKEY 99

struct msgform {
	int mtype;
	char mtext[1000];
}msg;

int  msgqid;

void server() {
	msgqid=msgget(MSGKEY, 0777|IPC_CREAT);	/*创建KEY为99的消息队列*/

    if(msgqid == -1){
        perror("msgget");
        printf("%d\n",errno);
        exit(1);
    }

	do{
		msgrcv(msgqid, &msg, sizeof(struct msgform), 0, 0);   /*接收消息*/
		printf("server received, %d\n",msg.mtype);
        sleep(1);
	}while(msg.mtype!=1);
	msgctl(msgqid, IPC_RMID, 0);			/*删除消息队列*/
	exit(0);
}

int main(){  
	server();
    return 0;
}