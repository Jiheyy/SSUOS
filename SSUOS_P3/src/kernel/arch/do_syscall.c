#include <proc/sched.h>
#include <proc/proc.h>
#include <device/device.h>
#include <interrupt.h>
#include <device/kbd.h>
#include <filesys/file.h>

pid_t do_fork(proc_func func, void* aux1, void* aux2)
{
	pid_t pid;
	struct proc_option opt;

	opt.priority = cur_process-> priority;
	pid = proc_create(func, &opt, aux1, aux2);

	return pid;
}

void do_exit(int status)
{
	cur_process->exit_status = status; 	//종료 상태 저장
	proc_free();						//프로세스 자원 해제
	do_sched_on_return();				//인터럽트 종료시 스케줄링
}

pid_t do_wait(int *status)
{
	while(cur_process->child_pid != -1)
		schedule();
	//SSUMUST : schedule 제거.

	int pid = cur_process->child_pid;
	cur_process->child_pid = -1;

	extern struct process procs[];
	procs[pid].state = PROC_UNUSED;

	if(!status)
		*status = procs[pid].exit_status;

	return pid;
}

void do_shutdown(void)
{
	dev_shutdown();
	return;
}

int do_ssuread(void)
{
	return kbd_read_char();
}

int do_open(const char *pathname, int flags)
{
	struct inode *inode;
	struct ssufile **file_cursor = cur_process->file;
	int fd;

	for(fd = 0; fd < NR_FILEDES; fd++)
		if(file_cursor[fd] == NULL) break;

	if (fd == NR_FILEDES)
		return -1;

	if ( (inode = inode_open(pathname, flags)) == NULL)
		return -1;
	
	if (inode->sn_type == SSU_TYPE_DIR)
		return -1;

	fd = file_open(inode,flags,0);
	
	return fd;
}

int do_read(int fd, char *buf, int len)
{
	return generic_read(fd, (void *)buf, len);
}
int do_write(int fd, const char *buf, int len)
{
	return generic_write(fd, (void *)buf, len);
}

int do_fcntl(int fd, int cmd, long arg)
{
	int flag = -1;
	struct ssufile **file_cursor = cur_process->file;


	if (cmd & F_DUPFD){

			int i;

			for(i=arg; file_cursor[i] != NULL ; i++); //arg값의 fd를 가지고 있는 지 확인 있다면 arg++해서 다시 확인해주기

			file_cursor[i] = file_cursor[fd]; //없다면 빈 사용가능한 fd값 i의 file을 fd의 file에 복붙


		return i; //붙여넣은 새 fd값 리턴
		
	}
	else if (cmd & F_GETFL){

			int val;

			val = file_cursor[fd]->flags;

			return val;
		
	}
	else if(cmd & F_SETFL){

			if(arg & O_APPEND) {
				file_cursor[fd]->flags = arg;  // arg 상태 플래그 값 추가
				file_cursor[fd]->pos = file_cursor[fd]->inode->sn_size;//cursor 위치 맨 마지막으로 이동	
				return file_cursor[fd]->flags;
			}

			return -1;

	}
	else{
	
			return -1;
	}

}
