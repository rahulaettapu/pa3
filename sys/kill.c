/* kill.c - kill */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <mem.h>
#include <io.h>
#include <q.h>
#include <stdio.h>
#include <lock.h>

/*------------------------------------------------------------------------
 * kill  --  kill a process and remove it from the system
 *------------------------------------------------------------------------
 */

void release_locks(int pid){
	int i;
	for(i=0;i<50;i++){
		if(proctab[pid].locktype[i] == READ || proctab[pid].locktype[i] == WRITE)
			release(i);
	}
}


void revert_changes(int pid){
	struct lentry *lptr;
	struct pentry *ptr;
	int i,j;
	int max = -1000;
	int max_pid = -1;
	int next;
	ptr = &proctab[pid];
	lptr = &ltab[ptr->lock_wait];

	//kprintf("called revert_change\n");	
	change_prio =1;
	prio_inv_update(pid,ptr->lock_wait,ptr->pprio);
	change_prio = 0;
	get_max_prio(ptr->lock_wait);

/*	kprintf("after killerset %d\n",ptr->pprio);
	int pp = q[lptr->lqhead].qnext;
	kprintf("hhh :%d\n", proctab[pp].pprio);
	while(pp != lptr->lqtail)
		kprintf("%d ,old prio : %d\t", proctab[pp].pprio,proctab[pp].old);
	kprintf("\n");*/
//	kprintf("%d : llptr lprio\n",lptr->lprio);	
	if(ptr->pprio == lptr->lprio){
//			kprintf("entered the equal condition of highest priority in lq\n");
			next = q[lptr->lqhead].qnext;
			while(next != lptr->lqtail){
			
		if(max < proctab[next].pprio && next != pid){
							max = proctab[next].pprio;
							max_pid = next;
					}
					next = q[next].qnext;
			}
//			kprintf("max: %d and max_pid is %d, pid is : %d \n",max,max_pid,pid);
			if(max_pid != -1)
				prio_inv_update(max_pid,ptr->lock_wait,max);
	}
	
	else{
		next = q[lptr->lqhead].qnext;
		while(next != lptr->lqtail){
			if(max < proctab[next].pprio && next!= pid){
					max = proctab[next].pprio;
					max_pid = next;
				}
			next = q[next].qnext;
		}
		if(max_pid!= -1)
			prio_inv_update(max_pid,ptr->lock_wait,max);
	}

	
}	
					

SYSCALL kill(int pid)
{
	STATWORD ps;    
	struct	pentry	*pptr;		/* points to proc. table for pid*/
	int	dev;
	int ldesc;

	disable(ps);
	if (isbadpid(pid) || (pptr= &proctab[pid])->pstate==PRFREE) {
		restore(ps);
		return(SYSERR);
	}
	if (--numproc == 0)
		xdone();

	dev = pptr->pdevs[0];
	if (! isbaddev(dev) )
		close(dev);
	dev = pptr->pdevs[1];
	if (! isbaddev(dev) )
		close(dev);
	dev = pptr->ppagedev;
	if (! isbaddev(dev) )
		close(dev);
		
	send(pptr->pnxtkin, pid);
	//proctab[pid].lock_wait = -1;
	freestk(pptr->pbase, pptr->pstklen);
	switch (pptr->pstate) {

	case PRCURR://	release_locks(pid);
			pptr->pstate = PRFREE;
			//release_locks(pid);	/* suicide */
			resched();

	case PRWAIT:	dequeue(pid);
			revert_changes(pid);
			proctab[pid].lock_wait = -1;
			semaph[pptr->psem].semcnt++;
	//		resched();

	case PRREADY:	dequeue(pid);
			release_locks(pid);
			pptr->pstate = PRFREE;
			break;

	case PRSLEEP:
	case PRTRECV:	unsleep(pid);
						/* fall through	*/
	default:	release_locks(pid);
			pptr->pstate = PRFREE;
	}
	restore(ps);
	return(OK);
}
