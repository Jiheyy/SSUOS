#include <filesys/inode.h>
#include <proc/proc.h>
#include <device/console.h>
#include <mem/palloc.h>

#include <device/block.h>

int file_open(struct inode *inode, int flags, int mode)
{
	int fd;
	struct ssufile **file_cursor = cur_process->file;

	for(fd = 0; fd < NR_FILEDES; fd++)
	{
		if(file_cursor[fd] == NULL)
		{
			if( (file_cursor[fd] = (struct ssufile *)palloc_get_page()) == NULL)
				return -1;
			break;
		}	
	}
	
	inode->sn_refcount++;

	file_cursor[fd]->inode = inode;
	file_cursor[fd]->pos = 0;

	//fcntl(fd, F_GETFL);

	if(flags & O_APPEND){

			cur_process->file[fd]->pos = inode->sn_size;

	}
	else if(flags & O_TRUNC){


			char * buf = " ";
			uint32_t size = file_cursor[fd]->inode->sn_size; //file cursor 의 값을 파일의 크기로 설정
			cur_process->file[fd]->pos = 0; //file cursor를 맨 앞으로 이동시킴.
			inode->sn_size = 0;

			//공백으로 덮어쓰고
		

			inode_write(inode, file_cursor[fd]->pos, buf, size);
	}

	file_cursor[fd]->flags = flags;
	file_cursor[fd]->unused = 0;

	return fd;
}

int file_write(struct inode *inode, size_t offset, void *buf, size_t len)
{
	return inode_write(inode, offset, buf, len);
}

int file_read(struct inode *inode, size_t offset, void *buf, size_t len)
{
	return inode_read(inode, offset, buf, len);
}


int file_close(uint32_t fd) {
}
