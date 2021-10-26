#include <list.h>
#include <proc/sched.h>
#include <mem/malloc.h>
#include <proc/proc.h>
#include <proc/switch.h>
#include <interrupt.h>

extern struct list plist;
extern struct list rlist;
extern struct list runq[RQ_NQS];
extern struct process procs[PROC_NUM_MAX];
extern struct process *idle_process;
struct process *latest;

bool more_prio(const struct list_elem *a, const struct list_elem *b,void *aux);
int scheduling; 					// interrupt.c

struct process* get_next_proc(void) 
{
	bool found = false;
	struct process *next = NULL;
	struct list_elem *elem;

	/* 
	   You shoud modify this function...
	   Browse the 'runq' array 
	*/
	int i=0;
		for(int i=0; i < RQ_NQS; i++)
			for(elem = list_begin(&runq[i]); elem != list_end(&runq[i]); elem = list_next(elem)) {
				struct process *p = list_entry(elem, struct process, elem_stat);
				if(p->state == PROC_RUN) {
					return p;
				}

		}
		return next;
				

	/*for(elem = list_begin(&rlist); elem != list_end(&rlist); elem = list_next(elem))
	{
		struct process *p = list_entry(elem, struct process, elem_stat);

		if(p->state == PROC_RUN)
			return p;
	}

	return next;*/
}

void schedule(void)
{
	struct process *cur;
	struct process *next;
	bool first = false;
	int pid;
	struct process *p;
	int i, times = 0;

	enum intr_level old_level;
	/* You shoud modify this function.... */
	old_level = intr_disable();
	proc_wake();
	
	unsigned long cur_ticks = get_ticks();

	next = get_next_proc();
	cur = cur_process;
	if(cur->pid != 0){
		next = idle_process;
		cur_process = next;
	}
	else {

			struct list_elem *e;
			for(e = list_begin(&plist); e != list_end(&plist); e = list_next(e)) {
					p = list_entry(e, struct process, elem_all);

						if (p -> pid == 0) // 0번이면 돌리기
								continue;
						
						if (p -> state == PROC_RUN) { // 프로세스 상태가 RUN인것을 출력한다.
							if (first == false) {
								printk("#= %d p= %3d c= %2d u= %3d", p -> pid, p-> priority, p -> time_slice, p -> time_used);
								first = true;
							}
							else {
								printk(", #= %d p= %3d c= %2d u= %3d", p -> pid, p-> priority, p -> time_slice, p -> time_used);
							}
						}

								
			}
			if(next->pid != 0)
			printk("\nSelected # = %d\n", next->pid);
	}
	cur_process = next;
	cur_process->time_slice = 0;
	switch_process(cur, next);
	if(next -> state == PROC_STOP && next->time_slice %60 == 0) {
		printk("Proc %d I/O at %d\n", next->pid, next->time_used);
	}
	intr_enable();

}
