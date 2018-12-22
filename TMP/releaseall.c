#include<conf.h>
#include<kernel.h>
#include<proc.h>
#include<q.h>
#include<lock.h>
#include<stdio.h>

extern unsigned long ctr1000;

void change_all_prio(int ldes){
		int i;
		int max = -1000;
		struct pentry *ptr;
		ptr = &proctab[currpid];
		for(i=0;i<NLOCK;i++){
			if(proctab[currpid].locktype[i] == READ || proctab[currpid].locktype[i] == WRITE){
					if(i != ldes){
						if(max < ltab[i].lprio)
								max = ltab[i].lprio;
					}
			}
		}
		
		if (max < ptr->old || max <0)
				ptr->pprio = ptr->old;
				
		else
				ptr->pprio = max;
}


int releaseall(int numlocks, int ldes1, ...){
	STATWORD(ps);
	int ldes;
	int retval = OK;
	int i = 0;
	disable(ps);

	for(i=0;i<numlocks;i++){
		ldes = (int)*((&ldes1) + i);
		retval = release(ldes);
	}

	resched();
	restore(ps);
	return retval;
}

int release(int ldes){

	struct pentry *ptr;
	struct lentry *lptr;

	lptr = &ltab[ldes];
	ptr = &proctab[currpid];

	int pid;
	int prev_pid;
	int prev_pid1;
	int write_flag =0;

	if(proctab[currpid].locktype[ldes] == DELETED  || lptr->lstate == LFREE)
		return SYSERR;

	if(lptr->hlock_proc[currpid] == NONE)
		return SYSERR;

	if(ptr->locktype[ldes] == READ)
		lptr->lreaders--;

	pid = q[lptr->lqtail].qprev;
//	kprintf("value returned from queue in release is %d and pid is %d\n",q[pid].qkey,pid);
//	pid = currpid;

	if(pid == lptr->lqhead){
	//	kprintf("entered empty \n");
		ptr->locktype[ldes] = NONE;
		lptr->ltype = NONE;
		lptr->lprio = ptr->pprio;
		lptr->hlock_proc[currpid] =NONE;
		change_all_prio(ldes);
		resched();
		return OK;
	}

	if(lptr->lreaders == 0){

		if(proctab[pid].locktype[ldes] == WRITE){
			ltab[ldes].ltype = WRITE;
			proctab[pid].lock_wait = -1;
			proctab[pid].plwaitingtime = ctr1000 - proctab[pid].plwaitingtime;
			lptr->hlock_proc[pid] = WRITE;
			write_flag =1;
	//		kprintf("eneterd write release check\n");
			dequeue(pid);
			ready(pid,RESCHNO);
	//		inherit_update(currpid);
		//	priority_inv_update(ldes);
			get_max_prio(ldes);
			change_all_prio(ldes);
			resched();
			
		}
		
		else if(proctab[pid].locktype[ldes] == READ){
//			kprintf("entered the read condition\n");
			prev_pid = q[pid].qprev;
			prev_pid1 = prev_pid;
//			kprintf("prev_pid is %d , pid is %d\n",prev_pid,pid);
			while((prev_pid != lptr->lqhead) && (q[prev_pid].qkey == q[pid].qkey)){
				
				if(proctab[pid].plwaitingtime - proctab[prev_pid].plwaitingtime < 1000){
						if(proctab[prev_pid].locktype[ldes] == WRITE){
								write_flag =1;
								ltab[ldes].ltype = WRITE;
								proctab[prev_pid].lock_wait = -1;
								proctab[prev_pid].plwaitingtime = ctr1000-proctab[prev_pid].plwaitingtime;
								lptr->hlock_proc[prev_pid] = WRITE;
								dequeue(prev_pid);
								ready(prev_pid,RESCHNO);
						//		priority_inv_update(ldes);
						//		inherit_update(currpid);
								get_max_prio(ldes);
								change_all_prio(ldes);				
								resched();
								
						}
				}
				prev_pid = q[prev_pid].qprev;
			}
			
			if(write_flag == 0){
//				kprintf("entered write_flag=0\n");
				//prev_pid = q[pid].qprev;
				int max_write = -1000;
				while(prev_pid1 != lptr->lqhead){
					if(proctab[prev_pid1].locktype[ldes] == WRITE){
						max_write = q[prev_pid1].qkey;
						break;
					}
					prev_pid1 = q[prev_pid1].qprev;
				}
			//	kprintf("max key is %d\n",max_write);

				if(max_write < 0){
					prev_pid1 = q[pid].qprev;
					while(prev_pid1 != lptr->lqhead && proctab[prev_pid1].locktype[ldes] == READ){
						ltab[ldes].ltype = READ;
						proctab[prev_pid1].lock_wait = -1;
						lptr->hlock_proc[prev_pid1] = READ;
						proctab[prev_pid1].plwaitingtime = ctr1000 - proctab[prev_pid1].plwaitingtime;
						lptr->lreaders++;
						dequeue(prev_pid1);
						ready(prev_pid1,RESCHNO);
						prev_pid1 = q[prev_pid1].qprev;
					}
					lptr->lreaders++;
					proctab[pid].lock_wait = -1;
					lptr->hlock_proc[pid] = READ;
					dequeue(pid);
					ready(pid,RESCHNO);
				}

				else{
					prev_pid1 = q[pid].qprev;
			//		kprintf("prev_pid11 is %d in max_write > 0 case\n",prev_pid1);
					while(!(prev_pid1 > NPROC)){
						if(max_write <= q[prev_pid1].qkey && proctab[prev_pid1].locktype[ldes] == READ){
							ltab[ldes].ltype = READ;
							proctab[prev_pid1].lock_wait = -1;
							lptr->hlock_proc[prev_pid1] = READ;
						proctab[prev_pid1].plwaitingtime = ctr1000 - proctab[prev_pid1].plwaitingtime;

                                                lptr->lreaders++;
                                                dequeue(prev_pid1);
                                                ready(prev_pid1,RESCHNO);
						}
                                                prev_pid1 = q[prev_pid1].qprev;
					}
					lptr->lreaders++;
					proctab[pid].lock_wait = -1;
					lptr->hlock_proc[pid] = READ;
					dequeue(pid);
					ready(pid,RESCHNO);
				}
				get_max_prio(ldes);
				change_all_prio(ldes);
				resched();
			}

			/*	while(prev_pid1 != lptr->lqhead && q[prev_pid1].qkey <= q[pid].qkey && proctab[prev_pid1].locktype[ldes] == READ){
				//	kprintf("shud not eneter\n");
					if(proctab[prev_pid1].locktype[ldes] == WRITE)
							break;
					else if(proctab[prev_pid1].locktype[ldes] == READ){
						ltab[ldes].ltype = READ;
						proctab[prev_pid1].lock_wait = -1;
						lptr->hlock_proc[prev_pid1] = READ;
						proctab[prev_pid1].plwaitingtime = ctr1000 - proctab[prev_pid1].plwaitingtime;
						lptr->lreaders++;
						dequeue(prev_pid1);
						ready(prev_pid1,RESCHNO);
					}
					prev_pid1 = q[prev_pid1].qprev;
				}
				lptr->lreaders++;
				proctab[pid].lock_wait = -1;
				lptr->hlock_proc[pid] = READ;

				dequeue(pid);
				ready(pid,RESCHNO);
			//	inherit_update(currpid);
				get_max_prio(ldes);
				change_all_prio(ldes);
				resched();
				

			}*/
	}
	}
	return proctab[currpid].pwaitret;

}
			 
						
			
			
			
