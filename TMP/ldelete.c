#include<conf.h>
#include<kernel.h>
#include<proc.h>
#include<q.h>
#include<lock.h>
#include<stdio.h>

int ldelete(int ldes){
	int pid;
	struct lentry *lptr;
	struct pentry *ptr;
	lptr = &ltab[ldes];
	ptr = &proctab[currpid];
	if(ltab[ldes].lstate == LFREE || ltab[ldes].lstate == DELETED || ptr->locktype[ldes] == NONE)
		return SYSERR;
	
	lptr->lstate = DELETED;
	lptr->ltype = NONE;
	lptr->lreaders = 0;
	if(nonempty(lptr->lqhead)){
		while((pid = getfirst(lptr->lqhead))!= EMPTY){
			proctab[pid].pwaitret = DELETED;
			proctab[pid].locktype[ldes] = NONE;
			ready(pid,RESCHNO);
		}
		resched();
	}
	return proctab[pid].pwaitret;
}
			
