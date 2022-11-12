#include<stdio.h>
#include <stdlib.h>

#include "data.h"

int main(){
    struct status ss1,ss2;
    int i;
    init_status(&ss1);
    ss1.s_bullets = malloc(3*sizeof(struct pos));
    ss1.s_bullsize = 3;
    ss1.s_bullets[1].c = 1;
    for(i=0;i<ss1.s_bullsize;i++){
        printf("%d ", ss1.s_bullets[i].c);
    }
    ss2.o_bullets = malloc(2*sizeof(struct pos));
    get_reverse(&ss2, &ss1);
    for(i=0;i<ss2.o_bullsize;i++){
        printf("%d ", ss2.o_bullets[i].c);
    }
    printf("\n");
    return 0;
}