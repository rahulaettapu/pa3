#include<conf.h>
#include<kernel.h>
#include<q.h>
#include<proc.h>
#include<lock.h>
#include<stdio.h>

extern unsigned long ctr1000;
/*void inherit_update(int pid){
	int max_q = get_max_prio_q(pid);
	if(max_q < proctab[pid].pinh)
		proctab[pid].pprio = proctab[pid].pinh;
	else
		proctab[pid].pprio = max_q;
	
	if(proctab[pid].lock_wait!= -1){
		priority_inv_update(proctab[pid].lock_wait);
	}
}

int get_max_prio_q(int pid){
	int i;
	int max = -1000;
	for(i=0;i<50;i++){
		if(proctab[pid].locktype[i] !=NONE){
			int max_1 = get_max_prio(i);
			if(max < max_1)
				max = max_1;
		}
	}
}
*/
void get_max_prio(int ldes1){
	int i,j;
	int max = -1000;
	for(i=0;i<NPROC;i++){
		if(proctab[i].lock_wait == ldes1){
			if(max < proctab[i].pprio)
				max = proctab[i].pprio;
		}
	}
	
/*	int p = q[ltab[ldes1].lqtail].qprev;
	while(p!=ltab[ldes1].lqhead){
		if(max < proctab[p].pprio){
			max = proctab[p].pprio;
		}
	}*/
	ltab[ldes1].lprio = max;
	//return max;
}



void prio_inv_update(int pid,int ldes,int newprio)
	{
	struct pentry *ptr;
	struct lentry *lptr;
	int i;
	int wl;
	ptr = &proctab[pid];
	lptr = &ltab[ldes];
	
	if(change_prio){
		if(ptr->pinh ==1){
			//kprintf("entered change_prio \n");
			ptr->pinh=0;
			ptr->pprio = ptr->old;
		}
	}
	//	kprintf("present prio : %d and newprio %d\n",ptr->pprio,newprio);		
	else{	if(ptr->pprio < newprio){
				ptr->pinh =1;
				ptr->pprio = newprio;
		}
	}	
//		if(ptr->pstate == PFREE)
//			return;
			
		if(ptr->lock_wait == -1)
			return;
			
		for(i=0;i<50;i++){
			if(lptr->hlock_proc[i] == READ || lptr->hlock_proc[i] == WRITE){
				wl = proctab[i].lock_wait;
				prio_inv_update(i,wl,newprio);
			}
		}
	}



int lock(int ldes1, int type, int priority){

	struct lentry *lptr;
	struct pentry *ptr;

	lptr = &ltab[ldes1];

	int curr = q[lptr->lqhead].qnext;
/*	while(curr != lptr->lqtail){
		kprintf("%d\t",proctab[curr].pprio);
		curr = q[curr].qnext;
	}
	kprintf("\n");
*/	
	if(lptr->lstate == LFREE || proctab[currpid].locktype[ldes1] == DELETED)
		return SYSERR;
	
	if(lptr->lstate == LUSED && lptr->ltype == NONE){
		lptr->ltype = type;
		if(type == READ){
			proctab[currpid].locktype[ldes1] = READ;
			lptr->lreaders++;
			lptr->hlock_proc[currpid] = READ;
			lptr->ltype = READ;
		}
		else{
			proctab[currpid].locktype[ldes1] = WRITE;
			lptr->hlock_proc[currpid] = WRITE;
			lptr->ltype = WRITE;
		}
		
		
	}

	else if( lptr->ltype == WRITE){
		ptr = &proctab[currpid];
		if(type == READ){
			ptr->locktype[ldes1] = READ;
			
		}
		else
			ptr->locktype[ldes1] = WRITE;

		ptr->pstate = PRWAIT;
		ptr->plwaitingtime = ctr1000;
		ptr->lock_wait = ldes1;
		insert(currpid,lptr->lqhead,priority);
		
/* int curr = q[lptr->lqhead].qnext;
        while(curr != lptr->lqtail){
                kprintf("%d\t",proctab[curr].pprio);
                curr = q[curr].qnext;
        }
        kprintf("\n");

*/
		get_max_prio(ldes1);
		prio_inv_update(currpid,ldes1,proctab[currpid].pprio);
		get_max_prio(ldes1);
//		kprintf(" lock queue max :%d \n",lptr->lprio);
		resched();
		
	}

	else if(lptr->ltype == READ){
		ptr = &proctab[currpid];
		if(type == READ){
			ptr->locktype[ldes1] = READ;
			int max = q[lptr->lqtail].qprev;
			max = q[max].qkey;
			
			if(max <= priority || max > NPROC){
				lptr->lreaders++;
				lptr->hlock_proc[currpid] = READ;
				proctab[currpid].locktype[ldes1] = READ;
				
				
			}

			else if(max > priority){

				ptr->pstate = PRWAIT;
				ptr->plwaitingtime = ctr1000;
				ptr->lock_wait = ldes1;
		//		kprintf("about to insert\n");
				insert(currpid,lptr->lqhead,priority);

				int check = q[lptr->lqhead].qnext;
			//	kprintf("%d \n", q[check].qkey);
				get_max_prio(ldes1);
				prio_inv_update(currpid,ldes1,proctab[currpid].pprio);
				get_max_prio(ldes1);
				resched();
				
			}
		}

		else {
			ptr->locktype[ldes1] = WRITE;
			ptr->pstate = PRWAIT;
			ptr->plwaitingtime = ctr1000;
			ptr->lock_wait = ldes1;
			insert(currpid,lptr->lqhead,priority);
			get_max_prio(ldes1);
			prio_inv_update(currpid,ldes1,proctab[currpid].pprio);
			get_max_prio(ldes1);
			resched();
			
		}
	}

	return proctab[currpid].pwaitret;

}			
					

	
