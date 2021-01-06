#include<stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "data.h"
#include "graph.h"
#include <curses.h>

int main(){
    // struct status ss1,ss2;
    // int i;
    // init_status(&ss1);
    // ss1.s_bullets = malloc(3*sizeof(struct pos));
    // ss1.s_bullsize = 3;
    // ss1.s_bullets[1].c = 1;
    // for(i=0;i<ss1.s_bullsize;i++){
    //     printf("%d ", ss1.s_bullets[i].c);
    // }
    // ss2.o_bullets = malloc(2*sizeof(struct pos));
    // get_reverse(&ss2, &ss1);
    // for(i=0;i<ss2.o_bullsize;i++){
    //     printf("%d ", ss2.o_bullets[i].c);
    // }

    // printf("\n");
    // printf("%d",LINES);
    // int l = LINES;
    // printf("%d", l-1);
    // printf("%d", -1+l);


	initscr();			/* turn on curses	*/
    clear();			/* draw some stuff	*/
    
    struct status sts;
    init_status(&sts);

    // draw_status(&sts,NULL,-1);
    // draw_other(1, 1, 2,2);
    // draw_self(LINES-4, COLS-10, 2,2);
    struct pos *bulls = sts.o_bullets;
    bulls[0].l = 0;
    bulls[0].c = 4;
    sts.o_bullsize = 1;
    draw_bullets(bulls,1);

    refresh();
    pause();
    getch();			/* wait for user input	*/
    getch();			/* wait for user input	*/
	endwin();			/* reset the tty etc	游戏退出时调用*/
    printf("123\n");
    return 0;
}