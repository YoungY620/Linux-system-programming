#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>  // shmat
#include <sys/shm.h>    // shmat shmget
#include <sys/ipc.h>    // shmget
#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <semaphore.h>       // sem_open

#define MAX_E 6
#define DATA_KEY 99
#define CNTR_KEY 100

struct element{
    int a;
    float b;
};
struct element *l;
int *e_cntr;

void consume();
void produce();
void print_products(struct element *elems, int size);

int main(int ac, char *av[]){
    int seg_id;
    int fkrv;
    sem_t *mutex;

    l = malloc(sizeof(struct element)*MAX_E);
    e_cntr = malloc(sizeof(int));
    (*e_cntr) = 0;

    seg_id = shmget(DATA_KEY, sizeof(struct element)*MAX_E, IPC_CREAT|0777);
    l = shmat(seg_id, NULL, 0);
    seg_id = shmget(CNTR_KEY, sizeof(int), IPC_CREAT|0777);
    e_cntr = shmat(seg_id, NULL, 0);

    if((mutex = sem_open("mutexsem", O_CREAT, 0644, 1)) == SEM_FAILED) {
        perror("unable to create semaphore");
        sem_unlink("mutexsem");
        exit(1);
    }
    
    if((fkrv=fork()) == 0){
        while(1){
            sem_wait(mutex);
            consume(e_cntr, l);
            print_products(l, *e_cntr);
            printf("consumer %d ", *e_cntr);
            sem_post(mutex);
            usleep(800+rand()*1000/RAND_MAX);
        }
    }else {
        while(1){
            sem_wait(mutex);
            produce(e_cntr, l);
            print_products(l, *e_cntr);
            printf("producer %d ",*e_cntr);
            sem_post(mutex);
            usleep(800+rand()*1000/RAND_MAX);
        }
    }

    return 0;
}
void produce(int *idx, struct element *elems){
    if((*idx) < MAX_E){
        elems[(*idx)].b = 2.3;
        elems[(*idx)++].a = 1;
    }
}
void consume(int *idx, struct element *elems){
    if((*idx) > 0){
        elems[(*idx)].b = 0;
        elems[(*idx)--].a = -1;
    }
}
void print_products(struct element *elems, int size){
    int i;
    for (i=0;i<size;i++){
        printf("%d-%4.2f ",elems[i].a, elems[i].b);
    }
    printf("\n");
}