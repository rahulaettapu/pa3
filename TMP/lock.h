#ifndef LOCK_H_
#define LOCK_H_

#define NLOCKS 50
#define READ 200
#define WRITE 201
#define NONE 202

#define LFREE '\01'
#define LUSED '\02'

struct lentry {
	char lstate;
	int ltype;
	int lreaders; // no of reader locks
	int hlock_proc[50];
	int lqhead;
	int lqtail;
	int lprio;
};

extern struct lentry ltab[NLOCKS];

extern void linit();
extern int lcreate();
extern int ldelete(int);
extern int lock(int,int,int);
extern int releaseall(int,int,...);
int change_prio;
#endif
