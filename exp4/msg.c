#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>

#define MSGKEY 99

void server();
void client();

struct msgform {
	int mtype;
	char mtext[1000];
};
int msgqid;

int main(int ac, char *av[]){
    int fpid;

    if(-1==(fpid=fork())){
        perror("fork");
        exit(1);
    }
    if(fpid==0){
        client();
        return 0;
    }else{
        server();
        if(-1==wait(NULL)){
            perror("wait");
            exit(EXIT_FAILURE);
        }
        return 0;
    }
}

void server() {
    struct msgform msg;
    msgqid=msgget(MSGKEY, 0777|IPC_CREAT);	/*创建KEY为99的消息队列*/
	do{
		if(-1==msgrcv(msgqid, &msg, sizeof(struct msgform), 0, 0)){
            perror("msgrcv");
            exit(EXIT_FAILURE);
        };   /*接收消息*/
		// printf("server received\n");
		printf("server received, ptr:%ld mtype:%d, text:%s\n",(long)&msg, msg.mtype, msg.mtext);
        sleep(1);
	}while(msg.mtype!=1);
	msgctl(msgqid, IPC_RMID, 0);			/*删除消息队列*/
	exit(0);
}

void client() {
	int i;
    struct msgform msg;
	msgqid=msgget(MSGKEY, 0777);		/*打开KEY为99的消息队列*/
	for(i=10; i>=1; i--) {
		msg.mtype=i;
		// printf("client sent\n");
		printf("client sent ptr:%ld, mtype:%d, text:%s\n", (long)&msg, msg.mtype, msg.mtext);
		if(-1==msgsnd(msgqid, &msg, sizeof(struct msgform), 0)){
            perror("msgsnd");
            exit(EXIT_FAILURE);
        };	/*发送消息*/
        sleep(1);
    }
	exit(0);
}