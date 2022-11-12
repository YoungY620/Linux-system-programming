#ifndef __GAME_H__
#define __GAME_H__

#include <stdio.h>
#include <stdlib.h>

#include "data.h"
/*
    游戏逻辑
    1. 每个固定周期（1s）移动一次所有子弹
    2. 每个周期检查是否可以补充弹药
    3. 当有操作的时候，移动平台，或增加子弹（如子弹重合不进行操作）
    4. 每次更新场景后判定有无减血
    5. 每次减血检查有无获胜
    获胜的话则server发出EXIT 3(success)/4(defeated)
*/
/*
    check
    1. if who can replenish bullets
    2. if anyone lose hp
    3. if anyone win. res_code: 0-0th win, 1-1st win, 2-both dead
*/
void check_health(struct status *stsa, int *res_code){
    int i;

    *res_code = -1;
    for(i=0; i<stsa->s_bullsize; i++){
        if(stsa->s_bullets[i].l == BLT_L_LWR 
        && stsa->s_bullets[i].c >= stsa->other.c
        && stsa->s_bullets[i].c < stsa->other.c + PL_WIDTH){
            // on target
            stsa->o_hp--;
        }
    }
    for(i=0; i<stsa->o_bullsize; i++){
        if(stsa->o_bullets[i].l == BLT_L_UPR
        && stsa->o_bullets[i].c >= stsa->self.c
        && stsa->o_bullets[i].c < stsa->self.c + PL_WIDTH){
            // on target
            stsa->s_hp--;
        }
    }
    if(stsa->s_hp <= 0 && stsa->o_hp <= 0){
        *res_code = 2;
    }else if(stsa->s_hp <= 0){
        *res_code = 1;
    }else if(stsa->o_hp <= 0){
        *res_code = 0;
    }
}
int refresh_check(struct status *stsa,struct status *stsb, int *res_code){
    if(stsa->s_nexte == 0){ 
        if(stsa->s_ep < MAX_EP){
            stsa->s_ep ++;
        }
        stsa->s_nexte = RELOAD_ITR;
    }
    if(stsa->o_nexte == 0){
        if(stsa->o_ep < MAX_EP){
            stsa->o_ep ++;
        }
        stsa->o_nexte = RELOAD_ITR;
    }
    check_health(stsa, res_code);
    return 0;
}
void bullet_step(struct pos **bs, int *size){
    int i;
    int v_ptr = 0;
    struct pos valid_bullets[*size];

    for(i=0;i<(*size);i++){
        if(--((*bs)[i].l) >= BLT_L_LWR){
            valid_bullets[v_ptr].l = (*bs)[i].l;
            valid_bullets[v_ptr++].c = (*bs)[i].c;
        }
    }
    free((*bs));
    (*bs) = NULL;
    (*bs) = malloc(sizeof(struct pos)*v_ptr);
    memcpy((*bs), valid_bullets, sizeof(struct pos)*v_ptr);
    (*size) = v_ptr;
}
/*
    固定周期
*/
int periodic_step(struct status *stsa,struct status *stsb, int *res_code){
    int i;
    bullet_step(&(stsa->s_bullets),&(stsa->s_bullsize));
    bullet_step(&(stsa->o_bullets),&(stsa->o_bullsize));

    if(stsa->s_nexte>0 && stsa->s_ep != MAX_EP) stsa->s_nexte --;
    if(stsa->o_nexte>0 && stsa->o_ep != MAX_EP) stsa->o_nexte --;

    refresh_check(stsa, stsb, res_code);

    if(-1 == get_reverse(stsb, stsa)){
        fprintf(stderr, "in game.h periodic step\n");
        return -1;
    }
    return 0;
}
void do_shoot(struct status *self){
    int i;
    struct pos tmp[self->s_bullsize+1];
    struct pos bullet;
    bullet.c = self->self.c+(int)(PL_WIDTH/2);
    bullet.l = BLT_L_UPR;
    // if there is bullets
    if(self->s_ep == 0){
        return;
    }
    // if there is bullet overlap
    for(i=0;i<self->s_bullsize;i++){
        if(self->s_bullets[i].l == bullet.l 
        && self->s_bullets[i].c == bullet.c){
            return;
        }
    }
    memcpy(tmp, self->s_bullets, self->s_bullsize*sizeof(struct pos));
    tmp[self->s_bullsize++] = bullet;
    free(self->s_bullets);
    self->s_bullets = malloc(self->s_bullsize*sizeof(struct pos));
    memcpy(self->s_bullets, tmp, sizeof(struct pos)*self->s_bullsize);
    self->s_ep --;
}
int opt_step(struct status *self, char opt,struct status *other, int *res_code){
    if(opt == 'L' && self->self.c>PL_C_LWR){
        self->self.c --;
    }else if(opt == 'R' && self->self.c < PL_C_UPR){
        self->self.c ++;
    }else if(opt == 'U' && self->self.l > PL_L_LWR){
        self->self.l --;
    }else if(opt == 'D' && self->self.l < PL_L_UPR){
        self->self.l ++;
    }else if(opt == 'S'){
        do_shoot(self);
    }
    if(-1==refresh_check(self, other, res_code)){
        fprintf(stderr, "opt_step\n");
        return -1;
    }
    get_reverse(other, self);
    return 0;
}

#endif