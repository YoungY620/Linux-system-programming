#ifndef __DATA_H__
#define __DATA_H__

#include <stdint.h>
#include <string.h>
#include <curses.h>

#define MAX_HP 2
#define MAX_EP 2
#define RELOAD_ITR 3

#define SCREEN_C 80
#define SCREEN_L 16

#define PL_HIGHT 2      // DO NOT CHANGE
#define PL_WIDTH 6      // DO NOT CHANGE
#define BLT_HIGHT 1     // DO NOT CHANGE
#define BLT_WIDTH 1     // DO NOT CHANGE

#define PL_C_UPR (SCREEN_C - PL_WIDTH)              // upper band
#define PL_L_UPR (SCREEN_L - PL_HIGHT)
#define PL_C_LWR 0                                  // lower band
#define PL_L_LWR 0

#define BLT_C_UPR (SCREEN_C - BLT_WIDTH)            // bullet coor upper band
#define BLT_L_UPR (SCREEN_L - BLT_HIGHT)
#define BLT_C_LWR 0                                 // lower
#define BLT_L_LWR 0

#define CENTRAL_SYMMETRY(V, LOW, UP) ((LOW) + (UP) - (V))

struct pos{
    int c,l;
};

struct status{
    struct pos self, other;
    int s_hp, o_hp;
    int s_ep, o_ep;
    int s_nexte, o_nexte;
    int s_bullsize;
    struct pos *s_bullets;
    int o_bullsize;
    struct pos *o_bullets;
};
/*
    TODO RISK: MEMORY OVERLAP
*/
int get_reverse(struct status *to, const struct status *from){
    int i;

    to->self.c = CENTRAL_SYMMETRY(from->other.c, PL_C_LWR, PL_C_UPR);
    to->self.l = CENTRAL_SYMMETRY(from->other.l, PL_L_LWR, PL_L_UPR);
    to->other.c = CENTRAL_SYMMETRY(from->self.c, PL_C_LWR, PL_C_UPR);
    to->other.l = CENTRAL_SYMMETRY(from->self.l, PL_L_LWR, PL_L_UPR);
    to->s_hp = from->o_hp; to->o_hp = from->s_hp;
    to->s_ep = from->o_ep; to->o_ep = from->s_ep;
    to->s_nexte = from->o_nexte;
    to->o_nexte = from->s_nexte;
    to->s_bullsize = from->o_bullsize;
    to->o_bullsize = from->s_bullsize;
    
    if(from->o_bullets != NULL){
        free(to->s_bullets);
        to->s_bullets = NULL;
        to->s_bullets = (struct pos *)malloc(sizeof(struct pos)*to->s_bullsize);
        memcpy(to->s_bullets, from->o_bullets, sizeof(struct pos)*to->s_bullsize);
    }
    if(from->s_bullets != NULL){
        free(to->o_bullets);
        to->o_bullets = NULL;
        to->o_bullets = (struct pos *)malloc(sizeof(struct pos)*to->o_bullsize);
        memcpy(to->o_bullets, from->s_bullets, sizeof(struct pos)*to->o_bullsize);
    }

    for(i=0;i<to->s_bullsize;i++){
        to->s_bullets[i].c = CENTRAL_SYMMETRY(from->o_bullets[i].c, BLT_C_LWR, BLT_C_UPR);
        to->s_bullets[i].l = CENTRAL_SYMMETRY(from->o_bullets[i].l, BLT_L_LWR, BLT_L_UPR);
    }
    for(i=0;i<to->o_bullsize;i++){
        to->o_bullets[i].c = CENTRAL_SYMMETRY(from->s_bullets[i].c, BLT_C_LWR, BLT_C_UPR);
        to->o_bullets[i].l = CENTRAL_SYMMETRY(from->s_bullets[i].l, BLT_L_LWR, BLT_L_UPR);
    }
    return 0;
}

void init_status(struct status *sts){
    sts->self.c = PL_C_UPR;
    sts->self.l = PL_L_UPR;
    sts->other.c = PL_C_LWR;
    sts->other.l = PL_L_LWR;
    sts->s_hp = MAX_HP;
    sts->o_hp = MAX_HP;
    sts->o_ep = MAX_EP;
    sts->s_ep = MAX_EP;
    sts->o_bullsize = 0;
    sts->s_bullsize = 0;
    sts->s_nexte = RELOAD_ITR;
    sts->o_nexte = RELOAD_ITR;
}

void data_to_str(const struct status* sts, char *dstr){
    int i;

    sprintf(dstr,"%03d %03d",sts->s_bullsize,sts->o_bullsize);
    sprintf(dstr,"%s %03d %03d",dstr, sts->self.c, sts->self.l);
    sprintf(dstr,"%s %03d %03d",dstr,sts->other.c,sts->other.l);
    sprintf(dstr,"%s %03d %03d",dstr,sts->s_hp,sts->o_hp);
    sprintf(dstr,"%s %03d %03d",dstr,sts->s_ep,sts->o_ep);
    sprintf(dstr,"%s %03d %03d",dstr,sts->s_nexte,sts->o_nexte);

    for(i=0;i<sts->s_bullsize;i++){
        sprintf(dstr,"%s %03d %03d",dstr,sts->s_bullets[i].c,sts->s_bullets[i].l);
    }
    for(i=0;i<sts->o_bullsize;i++){
        sprintf(dstr,"%s %03d %03d",dstr,sts->o_bullets[i].c,sts->o_bullets[i].l);
    }
}

int str_to_data(const char *str, struct status *sts,int s_size, int o_size){
    char *dstr = (char *)malloc(sizeof(char)*strlen(str));
    char *tok;
    int i;
    strcpy(dstr,str);

    tok=strtok(dstr," "); sts->self.c = atoi(tok);
    tok = strtok(NULL," "); sts->self.l = atoi(tok);
    tok = strtok(NULL," "); sts->other.c = atoi(tok);
    tok = strtok(NULL," ");  sts->other.l = atoi(tok);

    tok = strtok(NULL," "); sts->s_hp = atoi(tok);
    tok = strtok(NULL," "); sts->o_hp = atoi(tok);
    tok = strtok(NULL," "); sts->s_ep = atoi(tok);
    tok = strtok(NULL," "); sts->o_ep = atoi(tok);
    tok = strtok(NULL," "); sts->s_nexte = atoi(tok);
    tok = strtok(NULL," "); sts->o_nexte = atoi(tok);
    
    sts->s_bullsize = s_size;
    sts->o_bullsize = o_size;
    sts->s_bullets = malloc(sizeof(struct pos)*s_size);
    sts->o_bullets = malloc(sizeof(struct pos)*o_size);
    // if(s_size < sts->s_bullsize){
    //     fprintf(stderr,"not enough s_size. Found:%d %d %d\n", sts->s_bullsize, s_size, o_size);
    //     return -1;
    // }
    // if(o_size < sts->o_bullsize){
    //     fprintf(stderr,"not enough o_size.");
    //     return -1;
    // }
    for(i=0;i<sts->s_bullsize;i++){
        tok = strtok(NULL," "); sts->s_bullets[i].c = atoi(tok);
        tok = strtok(NULL," "); sts->s_bullets[i].l = atoi(tok);
    }
    for(i=0;i<sts->o_bullsize;i++){
        tok = strtok(NULL," "); sts->o_bullets[i].c = atoi(tok);
        tok = strtok(NULL," "); sts->o_bullets[i].l = atoi(tok);
    }
    free(dstr);

    return 0;
}

#endif
