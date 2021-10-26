#include <device/io.h>
#include <mem/mm.h>
#include <mem/paging.h>
#include <device/console.h>
#include <proc/proc.h>
#include <interrupt.h>
#include <mem/palloc.h>
#include <ssulib.h>
#include <mem/hashing.h>

uint32_t F_IDX(uint32_t addr, uint32_t capacity) {
    return addr % ((capacity / 2) - 1);
}

uint32_t S_IDX(uint32_t addr, uint32_t capacity) {
    return (addr * 7) % ((capacity / 2) - 1) + capacity / 2;
}

void init_hash_table(void)
{
//역페이지테이블 구현을 위한 테이블 초기화만 이루어지면 됨
//기존에 이미 구현된 페이징 기법과는 관련없음.
	// TODO : OS_P5 assignment

	for(int i=0; i< CAPACITY; i++)
		for(int j=0; j<SLOT_NUM; j++){
			hash_table.top_buckets[i].token[j] = 0;
			hash_table.top_buckets[i].slot[j].key = 0;
			hash_table.top_buckets[i].slot[j].value = 0;
		}

	for(int i=0; i< CAPACITY/2; i++)
		for(int j=0; j<SLOT_NUM; j++){
			hash_table.bottom_buckets[i].token[j] = 0;
			hash_table.bottom_buckets[i].slot[j].key = 0;
			hash_table.bottom_buckets[i].slot[j].value = 0;
		}


}

int insert_hash_table(void *pages, uint32_t key) {
		//key 는 page table 인덱스를 넣으면 된다.
	//*****삽입****
	//use two hash functions.
	//index
	int i;
	int newpos, expos;
	uint32_t value = (uint32_t)VH_TO_RH(pages);
	
	uint32_t top1 = F_IDX((uint32_t)pages, CAPACITY);
	uint32_t top2 = S_IDX((uint32_t)pages, CAPACITY);

	//삽입
	//check wheather there is a value in top1 pos
	for(i=0; i< SLOT_NUM; i++) {
		if(hash_table.top_buckets[top1].token[i] == 0) {
			hash_table.top_buckets[top1].slot[i].value = value;
			hash_table.top_buckets[top1].slot[i].key = key;
			hash_table.top_buckets[top1].token[i] = 1;
			printk("hash value inserted in top level : idx: %d, key: %d, value: %x\n", top1,key,value);
			return 1;
		}
		if(hash_table.top_buckets[top2].token[i] == 0) {
			hash_table.top_buckets[top2].slot[i].value = value;
			hash_table.top_buckets[top2].slot[i].key = key;
			hash_table.top_buckets[top2].token[i] = 1;
			printk("hash value inserted in top level : idx: %d, key: %d, value: %x\n", top2,key, value);
			return 1;
		}
	}
	//toplevel 버킷이 다 차있다면 대응하는 bottom-level 버킷에서 첫번째 해시함수의 결과 두번째 해시함수의 결과 확인 비어있는 위치에 저장
	int bottom1 = top1/2;
	int bottom2 = top2/2;
	for(i=0; i< SLOT_NUM; i++) {
		if(hash_table.bottom_buckets[bottom1].token[i] == 0) {
			hash_table.bottom_buckets[bottom1].slot[i].value = value;
			hash_table.bottom_buckets[bottom1].slot[i].key = key;
			hash_table.bottom_buckets[bottom1].token[i] = 1;
			printk("hash value inserted in bottom level : idx: %d, key: %d, value: %x\n", bottom1, key, value);
			return 1;
		}

		if(hash_table.bottom_buckets[bottom2].token[i] == 0) {
			hash_table.bottom_buckets[bottom2].slot[i].value = value;
			hash_table.bottom_buckets[bottom2].slot[i].key = key;
			hash_table.bottom_buckets[bottom2].token[i] = 1;
			printk("hash value inserted in bottom level : idx: %d, key: %d, value: %x\n", bottom2, key, value);
			return 1;
		}
	}

	//삽입 가능한 위치가 모두 차있다면  top-level버킷 아이템 중 하나를  다른  top-level버킷으로 이동 후 아이템 삽입.
	//새로운 버킷을 찾는 룰은 가상주소로 F_IDX, S_IDX를 적용한 값 중 현재 인덱스가 아닌 버킷에 삽입해야함을 의미.
	//top1
	uint32_t v;
	uint32_t k;
	uint32_t p; //새로운 가상주
	uint32_t f;
	uint32_t s;
	int slot;

	for(i=0; i<SLOT_NUM; i++) {
		if(hash_table.top_buckets[top1].token[i] == 1) {	
			v = hash_table.top_buckets[top1].slot[i].value;
			k = hash_table.top_buckets[top1].slot[i].key;
			p = RH_TO_VH(v); //value 를 가상주소로 변환
			f = F_IDX((uint32_t)p, CAPACITY); 
			s = S_IDX((uint32_t)p, CAPACITY);
			slot = i;
			break;
		}
	}

	if(f != top1) {
		//f로 옮기기
		//기존의 값 새로운 위치에 입력 후 기존 값 삭제
		for(int j=0; j<SLOT_NUM; j++) {
			if(hash_table.top_buckets[f].token[j] == 0) {
				hash_table.top_buckets[f].slot[j].value = v;
				hash_table.top_buckets[f].slot[j].key = k;
				hash_table.top_buckets[f].token[j] = 1;
				//기존값 삭제하는 거
				
				hash_table.top_buckets[top1].slot[slot].value = value;
				hash_table.top_buckets[top1].slot[slot].key = key;
				hash_table.top_buckets[top1].token[slot] = 1;
				printk("hash value inserted in top level : idx: %d, key: %d, value: %x\n",top1, key, value);
				return 1;
			}
		}
	}

	if(s != top1) {
		//s로 옮기기
		//기존의 값 새로운 위치에 입력 후 기존 값 삭제
		for(int j=0; j<SLOT_NUM; j++) {
			if(hash_table.top_buckets[s].token[j] == 0) {
				hash_table.top_buckets[s].slot[j].value = v;
				hash_table.top_buckets[s].slot[j].key = k;
				hash_table.top_buckets[s].token[j] = 1;
				
				//기존값 삭제하는 거
				hash_table.top_buckets[top1].slot[slot].value = value;
				hash_table.top_buckets[top1].slot[slot].key = key;
				hash_table.top_buckets[top1].token[slot] = 1;
				printk("hash value inserted in top level : idx: %d, key: %d, value: %x\n",top1, key, value);
				return 1;
			}
		}
	}
	
//top2에서 옮길 슬롯 고르기 및 데이타 저장, 새로운 해쉬값 구하기
	for(i=0; i<SLOT_NUM; i++) {
		if(hash_table.top_buckets[top2].token[i] == 1) {
			v = hash_table.top_buckets[top2].slot[i].value;
			k = hash_table.top_buckets[top2].slot[i].key;
			p = RH_TO_VH(v);
			f = F_IDX((uint32_t)p, CAPACITY);
			s = S_IDX((uint32_t)p, CAPACITY);
			slot = i;
			break;
		}
	}

	if(f != top2) {
		//f로 옮기기
		//기존의 값 새로운 위치에 입력 후 기존 값 삭제
		for(int j=0; j<SLOT_NUM; j++) {
			if(hash_table.top_buckets[f].token[j] == 0) {
				hash_table.top_buckets[f].slot[j].value = v;
				hash_table.top_buckets[f].slot[j].key = k;
				hash_table.top_buckets[f].token[j] = 1;
				//기존값 삭제하는 거
				
				hash_table.top_buckets[top2].slot[slot].value = value;
				hash_table.top_buckets[top2].slot[slot].key = key;
				hash_table.top_buckets[top2].token[slot] = 1;
				printk("hash value inserted in top level : idx: %d, key: %d, value: %x\n",top2, key, value);

				return 1;
			}
		}
	}

	if(s != top2) {
		//f로 옮기기
		//기존의 값 새로운 위치에 입력 후 기존 값 삭제
		for(int j=0; j<SLOT_NUM; j++) {
			if(hash_table.top_buckets[s].token[j] == 0) {
				hash_table.top_buckets[s].slot[j].value = v;
				hash_table.top_buckets[s].slot[j].key = k;
				hash_table.top_buckets[s].token[j] = 1;
				
				//기존값 삭제하는 거
				hash_table.top_buckets[top2].slot[slot].value = value;
				hash_table.top_buckets[top2].slot[slot].key = key;
				hash_table.top_buckets[top2].token[slot] = 1;
				printk("hash value inserted in top level : idx: %d, key: %d, value: %x\n",top2, key, value);

				return 1;
			}
		}
	}
		
	int b1;
	int b2;

	//top-level버킷 아이템 중 이동 가능한 아이템이 없다면 bottom_level 버킷 아이템 중 하나를 다른 bottom_level 버킷으로 이동 후 아이템 삽입
	//buttom2 빈 slot으로 이동
	//옮길 아이템 선택
	for(int j=0; j<SLOT_NUM; j++) {
		if(hash_table.bottom_buckets[bottom1].token[j] == 1 ) {
			v = hash_table.bottom_buckets[bottom1].slot[j].value;
			k = hash_table.bottom_buckets[bottom1].slot[j].key;
			p = RH_TO_VH(v);
			f = F_IDX((uint32_t)p, CAPACITY);
			s = S_IDX((uint32_t)p, CAPACITY);
			b1 = f/2;
			b2 = s/2;
			slot = j;
			break;
		}
	}

	if(b1 != bottom1) {
		//f로 옮기기
		//기존의 값 새로운 위치에 입력 후 기존 값 삭제
		for(int j=0; j<SLOT_NUM; j++) {
			if(hash_table.bottom_buckets[b1].token[j] == 0) {
				hash_table.bottom_buckets[b1].slot[j].value = v;
				hash_table.bottom_buckets[b1].slot[j].key = k;
				hash_table.bottom_buckets[b1].token[j] = 1;
				//기존값 삭제하는 거
				
				hash_table.bottom_buckets[bottom1].slot[slot].value = value;
				hash_table.bottom_buckets[bottom1].slot[slot].key = key;
				hash_table.bottom_buckets[bottom1].token[slot] = 1;
				printk("hash value inserted in bottom level : idx: %d, key: %d, value: %x\n",bottom1, key, value);
				return 1;
			}
		}
	}

	if(b2 != bottom1) {
		//s로 옮기기
		//기존의 값 새로운 위치에 입력 후 기존 값 삭제
		for(int j=0; j<SLOT_NUM; j++) {
			if(hash_table.bottom_buckets[b2].token[j] == 0) {
				hash_table.bottom_buckets[b2].slot[j].value = v;
				hash_table.bottom_buckets[b2].slot[j].key = k;
				hash_table.bottom_buckets[b2].token[j] = 1;
				
				//기존값 삭제하는 거
				hash_table.bottom_buckets[bottom1].slot[slot].value = value;
				hash_table.bottom_buckets[bottom1].slot[slot].key = key;
				hash_table.bottom_buckets[bottom1].token[slot] = 1;
				printk("hash value inserted in bottom level : idx: %d, key: %d, value: %x\n",bottom1, key, value);
				return 1;
			}
		}
	}
		

	for(int j=0; j<SLOT_NUM; j++) {
		if(hash_table.bottom_buckets[bottom2].token[j] == 1) {
			v = hash_table.bottom_buckets[bottom2].slot[j].value;
			k = hash_table.bottom_buckets[bottom2].slot[j].key;
			p = RH_TO_VH(v);
			f = F_IDX((uint32_t)p, CAPACITY);
			s = S_IDX((uint32_t)p, CAPACITY);
			b1 = f/2;
			b2 = f/2;
			slot = j;
			break;
		}
	}


	if(b1 != bottom2) {
		//f로 옮기기
		//기존의 값 새로운 위치에 입력 후 기존 값 삭제
		for(int j=0; j<SLOT_NUM; j++) {
			if(hash_table.bottom_buckets[b1].token[j] == 0) {
				hash_table.bottom_buckets[b1].slot[j].value = v;
				hash_table.bottom_buckets[b1].slot[j].key = k;
				hash_table.bottom_buckets[b1].token[j] = 1;
				//기존값 삭제하는 거
				
				hash_table.bottom_buckets[bottom2].slot[slot].value = value;
				hash_table.bottom_buckets[bottom2].slot[slot].key = key;
				hash_table.bottom_buckets[bottom2].token[slot] = 1;
				printk("hash value inserted in bottom level : idx: %d, key: %d, value: %x\n",bottom2, key, value);
				return 1;
			}
		}
	}

	if(b2 != bottom2) {
		//f로 옮기기
		//기존의 값 새로운 위치에 입력 후 기존 값 삭제
		for(int j=0; j<SLOT_NUM; j++) {
			if(hash_table.bottom_buckets[b2].token[j] == 0) {
				hash_table.bottom_buckets[b2].slot[j].value = v;
				hash_table.bottom_buckets[b2].slot[j].key = k;
				hash_table.bottom_buckets[b2].token[j] = 1;
				
				hash_table.bottom_buckets[bottom2].slot[slot].value = value;
				hash_table.bottom_buckets[bottom2].slot[slot].key = key;
				hash_table.bottom_buckets[bottom2].token[slot] = 1;
				printk("hash value inserted in bottom level : idx: %d, key: %d, value: %x\n",bottom2, key, value);
				return 1;
				//기존값 삭제하는 거
			}
		}
	}

	return 0;
}

int del_hash_table(void* pages, uint32_t key) {

	uint32_t top1 = F_IDX((uint32_t)pages, CAPACITY);
	uint32_t top2 = S_IDX((uint32_t)pages, CAPACITY);
	uint32_t value = (uint32_t)VH_TO_RH(pages); 
	int i;
	
	for(i=0; i< SLOT_NUM; i++) {
		if(hash_table.top_buckets[top1].slot[i].key == key) {
			hash_table.top_buckets[top1].token[i] = 0;
			hash_table.top_buckets[top1].slot[i].value = 0;
			hash_table.top_buckets[top1].slot[i].key = 0;
		   	printk("hash value deleted : idx: %d, key:  %d, value: %x\n", top1, key, value);
			return 1;
		}
	}
	for(i=0; i<SLOT_NUM; i++) {
		if(hash_table.top_buckets[top2].slot[i].key == key) {
			hash_table.top_buckets[top2].slot[i].value = 0;
			hash_table.top_buckets[top2].slot[i].key = 0;
			hash_table.top_buckets[top2].token[i] = 0;
			printk("hash value deleted : idx: %d, key: %d, value: %x\n", top2, key, value);
			return 1;
		}
	}
	int bottom1 = top1/2;
	int bottom2 = top2/2;
	for(i=0; i< SLOT_NUM; i++) {
		if(hash_table.bottom_buckets[bottom1].slot[i].key == key ) {
			hash_table.bottom_buckets[bottom1].slot[i].value = 0;
			hash_table.bottom_buckets[bottom1].slot[i].key = 0;
			hash_table.bottom_buckets[bottom1].token[i] = 0;
			printk("hash value deleted : idx: %d, key: %d, value: %x\n", bottom1, key, value);
			return 1;
		}
		if(hash_table.bottom_buckets[bottom2].slot[i].key == key ) {
			hash_table.bottom_buckets[bottom2].slot[i].value = 0;
			hash_table.bottom_buckets[bottom2].slot[i].key = 0;
			hash_table.bottom_buckets[bottom2].token[i] = 0;
			printk("hash value deleted : idx: %d, key: %d, value: %x\n", bottom2, key, value);
			return 1;
		}
	}

	
	return 0;

	
}
