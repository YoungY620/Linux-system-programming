#ifndef __GRAPH_H__
#define __GRAPH_H__

#include <stdio.h>
#include <stdlib.h>
#include <curses.h>

#include "data.h"

int draw_self(int l,int c, int hp, int ep){
    char hp_msg[100];
    char ep_msg[100];

    if(l < 0 || l >= LINES){
        fprintf(stderr,"Invalid arg \'l\'. Found:%d, Expected:[0,%d]",l,LINES-1);
        return -1;
    }
    if(c < 0 || c >= COLS){
        fprintf(stderr,"Invalid arg \'c\'. Found:%d, Expected:[0,%d]",c,COLS-1);
        return -1;
    }
    memset(ep_msg, 0, 100);
    memset(hp_msg, 0, 100);
    sprintf(hp_msg, "|__HP:%1d__|", hp);
    sprintf(ep_msg, "%02d", ep);

    move(l,c);
    standend();
    addstr(" __|");
    standout();
    addstr(ep_msg);
    standend();
    addstr("|__");
    move(l+1,c);
    addstr(hp_msg);
    // addstr("|________|");
    standend();

    // refresh();			/* update the screen	*/
    return 0;
}

int draw_other(int l, int c, int hp, int ep){
    char hp_msg[100];
    char ep_msg[100];

    if(l < 0 || l >= LINES){
        fprintf(stderr,"Invalid arg \'l\'. Found:%d, Expected:[0,%d]",l,LINES-1);
        return -1;
    }
    if(c < 0 || c >= COLS){
        fprintf(stderr,"Invalid arg \'c\'. Found:%d, Expected:[0,%d]",c,COLS-1);
        return -1;
    }
    sprintf(hp_msg, "|__HP:%1d__|", hp);
    sprintf(ep_msg, "%02d", ep);

    move(l+1,c);
    standend();
    addstr("   |");
    standout();
    addstr(ep_msg);
    standend();
    addstr("|  ");
    move(l,c);
    addstr(hp_msg);
    // addstr("|________|");
    standend();

    // refresh();			/* update the screen	*/
    return 0;
}

void draw_bullets(const struct pos *bullets, const int size){
    int i;
    standout();
    for(i=0;i<size;i++){
        move(bullets[i].l, bullets[i].c);
        addstr("*");
    }
    standend();
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
    
    // refresh();			/* update the screen	*/
    return 0;
}
int draw_status(struct status *sts, struct status *old, int log_fd){
    clear();
    standend();
    draw_bullets((const struct pos *)sts->s_bullets, sts->s_bullsize);
    draw_bullets((const struct pos *)sts->o_bullets, sts->o_bullsize);
    draw_self(sts->self.l, sts->self.c, sts->s_hp, sts->s_ep);
    draw_other(sts->other.l, sts->other.c, sts->o_hp, sts->o_ep);
    standend();
    refresh();
    if(log_fd != -1){
        int i;
        dprintf(log_fd, "%d o_bullets:\n", sts->o_bullsize);
        for (i=0;i<sts->o_bullsize;i++){
            dprintf(log_fd, "   %d %d\n", sts->o_bullets[i].c, sts->o_bullets[i].l);
        }
        dprintf(log_fd, "%d s_bullets:\n", sts->s_bullsize);
        for (i=0;i<sts->s_bullsize;i++){
            dprintf(log_fd, "   %d %d\n", sts->s_bullets[i].c, sts->s_bullets[i].l);
        }
    }
    return 0;
}

void show_end(const char *msg){
    clear();
    int l = (SCREEN_L/2);
    int c = ((SCREEN_C-strlen(msg))/2);
    move(l,c);
    addstr(msg);
    getch();
    refresh();
}
#endif