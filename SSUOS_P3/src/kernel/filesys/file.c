#include <filesys/inode.h>
#include <proc/proc.h>
#include <device/console.h>
#include <mem/palloc.h>

#include <device/block.h>

int file_seek(uint32_t fd, uint16_t pos) {

		void* buf;

		return generic_read(fd, buf, pos);



		//for(int i= 0; i<SECTORCOUNT(fs->fs_device) + pos; i++) {
		//		DEVOP_WRITE(fs->fs_device, startsec + i, buf + (i * fs->fs_device->blk_size));
		//}



	//	if(inode->sn_size < offset + len)
	//			inode->sn_size = offset+len;
	//	sync_inode(fs, inode);



	
	//char* buf = "MIN";
	//return inode_write(in , offset, buf, 4);

		//fs_readblock(fs, in->sn_directblock[blkoff], tmpblock);
}
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


	if(flags & O_APPEND){

			cur_process->file[fd]->pos = inode->sn_size;

	}
	else if(flags & O_TRUNC){


			//파일 내용을 삭제하는 방법이...
			//inode = inode_create(cur_process->cwd->sn_fs, SSU_TYPE_FILE);
			//file_cursor[fd]->inode = inode;
		//	file_cursor[fd]->pos = 0;
		//	inode->sn_refcount++;
			
		//	cur_process->file[fd]->inode = inode;
			const char * buf = " ";
			uint32_t size = file_cursor[fd]->inode->sn_size; //file cursor 의 값을 파일의 크기로 설정


			//공백으로 덮어쓰고
			//파일크기나 오프셋 변경
		

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
