#include<conf.h>
#include<kernel.h>
#include<proc.h>
#include<q.h>
#include<lock.h>
#include<stdio.h>

struct lentry ltab[NLOCKS];

void linit(){
	int i,j;
	struct lentry *lptr;
	for(i=0;i<NLOCKS;i++){
		(lptr = &ltab[i])->lstate = LFREE;
		lptr->ltype = NONE;
		lptr->lreaders = 0;
		for(j=0;j<50;j++){
			lptr->hlock_proc[j] = NONE;
		}
		lptr->lqtail = 1 + (lptr->lqhead = newqueue());
	}
}
