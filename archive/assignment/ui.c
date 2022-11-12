#include <stdio.h>
#include <stdlib.h>
#include <curses.h>
#include <unistd.h>

#define UP 72  
#define DWON 80
#define LEFT 75
#define RIGHT 77

int init_court();
int draw_plane(int l,int c);
int erase_plane(int l,int c);

int main()
{
    // todo 实现控制移动
    int ipt;
    int p_l = 0;
    int p_c = (COLS);
    p_c = p_c - 6;

	initscr();			/* turn on curses	*/
    clear();			/* draw some stuff	*/
    
    if(-1==init_court()){
        perror("init_court");
        exit(1);
    }
    printf("%d %d\n",LINES,COLS);
    draw_plane(0,0);

    getch();			/* wait for user input	*/
	endwin();			/* reset the tty etc	游戏退出时调用*/
    return 0;
}

int init_court(){
    int i;
    
    for(i=0; i<COLS; i++ ){		/* in a loop	*/
        move( LINES-1, i );
        if ( i%2 == 1 )
            standout();
        addstr(" ");
        if ( i%2 == 1 )
            standend();
    }
    for(i=0; i<LINES; i++ ){	/* in a loop	*/
        move( i, COLS-1 );
        if ( i%2 == 1 )
            standout();
        addstr(" ");
        if ( i%2 == 1 )
            standend();
    }
    
    refresh();			/* update the screen	*/
}


/*
    the plane img is:
            __
         __|  |__
        |________|
    hight == 2 lines
    width == 6 columns
    filled with standing-out white spaces 
*/
int draw_plane(int l,int c){
    if(l < 0 || l >= LINES){
        fprintf(stderr,"Invalid arg \'l\'. Found:%d, Expected:[0,%d]",l,LINES-1);
        return -1;
    }
    if(c < 0 || c >= COLS){
        fprintf(stderr,"Invalid arg \'c\'. Found:%d, Expected:[0,%d]",c,COLS-1);
        return -1;
    }
    move(l+1,c);
    standend();
    addstr(" __|");
    standout();
    addstr("  ");
    standend();
    addstr("|__");
    move(l,c);
    addstr("|__    __|");
    standend();

    refresh();			/* update the screen	*/
    return 0;
}
int erase_plane(int l,int c){
    if(l < 0 || l >= LINES){
        fprintf(stderr,"Invalid arg \'l\'. Found:%d, Expected:[0,%d]",l,LINES-1);
        return -1;
    }
    if(c < 0 || c >= COLS){
        fprintf(stderr,"Invalid arg \'c\'. Found:%d, Expected:[0,%d]",c,COLS-1);
        return -1;
    }
    move(l,c+2);
    standend();
    addstr("  ");
    move(l+1,c);
    addstr("      ");
    standend();
    
    refresh();			/* update the screen	*/
    return 0;
}