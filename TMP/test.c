#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <lock.h>
#include <stdio.h>
#include <lock.h>

#define DEFAULT_LOCK_PRIO 20


void reader22( char *msg, int lck)
{
/*	kprintf ("B reader difference does not make \n", msg);
		sleep(2);
	kprintf("B finished.\n");*/

	lock (lck, READ, DEFAULT_LOCK_PRIO);
	kprintf ("  %s: acquired lock, sleep 2s\n", msg);
	sleep (2);
	kprintf ("  %s: to release lock\n", msg);
	releaseall (1, lck);
}

void writer12 (char *msg, int lck)
{ 
	kprintf ("%s: Requesting the lock, sleep 2s\n", msg);
	lock (lck, WRITE, DEFAULT_LOCK_PRIO);
	kprintf ("%s: acquired the lock, sleep 2s\n", msg);
	sleep(1);
	kprintf ("%s: to release lock\n", msg);
	releaseall (1, lck);
}

void writer22 (char *msg, int sid)
{
	kprintf ("%s: Requesting the lock, sleep 2s\n", msg);
	wait(sid);
	kprintf ("%s: acquired the lock (semaphore), sleep 2s\n", msg);
	sleep(1);
	kprintf ("%s: to release lock\n", msg);
	signal(sid);
}

void test6 ()
{
	int lck;
	int pid1;
	int pid2;
	int pid3;
	int sid;

	kprintf("Using Lock Results \n");
	lck  = lcreate ();
	
	pid1 = create(writer12, 2000, 20, "Process a", 2, "A", lck);
	pid2 = create(reader22, 2000, 30, "Process b", 2, "B",lck);
	pid3 = create(writer12, 2000, 40, "Process c", 2, "C", lck);

	resume(pid1);
	resume(pid3);
	resume(pid2);
	sleep (15);
	ldelete (lck);

	kill(pid1);
	kill(pid3);
	kill(pid2);

	kprintf("Locks test done \n");
	
	kprintf("\n Using semaphores results \n");

	/* Using Semaphores */

	sid= screate(1);

	pid1 = create(writer22, 2000, 20, "Process a", 2, "A", sid);
	pid2 = create(reader22, 2000, 30, "Process b", 2, "B",sid);
	pid3 = create(writer22, 2000, 40, "Process c", 2, "C", sid);
	
	resume(pid1);
	resume(pid3);
	resume(pid2);	
	sleep (15);
	sdelete (sid);
	kprintf(" Done. \n");

}





