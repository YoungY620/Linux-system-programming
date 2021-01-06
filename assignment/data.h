#ifndef __DATA_H__
#define __DATA_H__

#include <stdint.h>
#include <string.h>

struct pos
{
    int c,l;
};
struct status
{
    struct pos self, other;
    int bull_size;
    struct pos *bullets;
};

void data_to_str(const struct status* sts, char *dstr){
    int i;

    sprintf(dstr,"%05d",sts->bull_size);
    sprintf(dstr,"%s %03d %03d",dstr, sts->self.c, sts->self.l);
    sprintf(dstr,"%s %03d %03d",dstr,sts->other.c,sts->other.l);
    for(i=0;i<sts->bull_size;i++){
        sprintf(dstr,"%s %03d %03d",dstr,sts->bullets[i].c,sts->bullets[i].l);
    }
}

int str_to_data(const char *str, struct status *sts,int cur_bull_size){
    char *dstr = malloc(sizeof(char)*strlen(str));
    char *tok;
    int i;
    strcpy(dstr,str);

    tok=strtok(dstr," ");
    sts->self.c = atoi(tok);
    tok = strtok(NULL," ");
    sts->self.l = atoi(tok);
    tok = strtok(NULL," ");
    sts->other.c = atoi(tok);
    tok = strtok(NULL," ");
    sts->other.l = atoi(tok);
    if(cur_bull_size < sts->bull_size){
        fprintf(stderr,"not enough bull_size.");
        return -1;
    }
    for(i=0;i<sts->bull_size;i++){
        tok = strtok(NULL," ");
        sts->bullets[i].c = atoi(tok);
        tok = strtok(NULL," ");
        sts->bullets[i].l = atoi(tok);
    }
    free(dstr);

    return 0;
}

#endif
