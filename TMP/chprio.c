/* chprio.c - chprio */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 * chprio  --  change the scheduling priority of a process
 *------------------------------------------------------------------------
 */
SYSCALL chprio(int pid, int newprio)
{
	STATWORD ps;    
	struct	pentry	*pptr;

	disable(ps);
	if (isbadpid(pid) || newprio<=0 ||
	    (pptr = &proctab[pid])->pstate == PRFREE) {
		restore(ps);
		return(SYSERR);
	}
	if(pptr->pstate == PRWAIT){
		if(newprio < pptr->pprio){
			revert_changes(pid);
			pptr->pprio = newprio;
			pptr->old = newprio;
		//	revert_changes(pid);
		}
		else{	
			prio_inv_update(pid,pptr->lock_wait,newprio);
			pptr->pprio = newprio;
			pptr->old = newprio;
		}
	}


	
	pptr->pprio = newprio;
	pptr->old = newprio;
	restore(ps);
	return(newprio);
}
