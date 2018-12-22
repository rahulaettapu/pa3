#include<conf.h>
#include<kernel.h>
#include<proc.h>
#include<q.h>
#include<lock.h>
#include<stdio.h>

LOCAL int newldes();


SYSCALL lcreate(){
	STATWORD ps;
	int ldes;
	disable(ps);
	if( (ldes = newldes()) == SYSERR){
		restore(ps);
		return SYSERR;
	}

	restore(ps);
	return(ldes);
}

LOCAL int newldes(){
	int i;
	for(i=0;i<NLOCKS;i++){
		if(ltab[i].lstate == LFREE){
			ltab[i].lstate = LUSED;
			ltab[i].ltype = NONE;
			proctab[currpid].locktype[i] = NONE;
			return(i);
		}
	}
	return (SYSERR);
}
