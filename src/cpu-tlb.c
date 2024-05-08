
/*
 * Copyright (C) 2024 pdnguyen of the HCMC University of Technology
 */
/*
 * Source Code License Grant: Authors hereby grants to Licensee 
 * a personal to use and modify the Licensed Source Code for 
 * the sole purpose of studying during attending the course CO2018.
 */
//#ifdef CPU_TLB
/*
 * CPU TLB
 * TLB module cpu/cpu-tlb.c
 */
 
#include "mm.h"
#include <stdlib.h>
#include <stdio.h>

int tlb_change_all_page_tables_of(struct pcb_t *proc,  struct memphy_struct * mp)
{
  /* TODO update all page table directory info 
   *      in flush or wipe TLB (if needed)
   */

  return 0;
}

int tlb_flush_tlb_of(struct tlb_cache *flush)
{
  /* TODO flush tlb cached*/
   struct tlb_node * ptr;
   printf("Flush cache:\n");
   while( flush->tlb_head != NULL){
	if(flush->tlb_head->writedflag == 1) 
		printf("memphy: %5d  memv: %5d  pid: %d\n",flush->tlb_head->memphy,flush->tlb_head->memvm,flush->tlb_head->pid);
	if(flush->tlb_head == flush->tlb_tail){
		flush->tlb_tail = NULL;
		free(flush->tlb_head);
		flush->tlb_head = NULL;
	}else{
		ptr = flush->tlb_head;
		flush->tlb_head = flush->tlb_head->next;
		free(ptr);
	}
   }
  
   while(flush->freelistHead != NULL){
	if(flush->freelistHead == flush->freelistTail){
		flush->freelistTail = NULL;
		free(flush->freelistHead);
		flush->freelistHead = NULL;
	}else{
		ptr = flush->freelistHead;
		flush->freelistHead = flush->freelistHead->next;
		free(ptr);
	}
   }
  
   return 0;
}

/*tlballoc - CPU TLB-based allocate a region memory
 *@proc:  Process executing the instruction
 *@size: allocated size 
 *@reg_index: memory region ID (used to identify variable in symbole table)
 */
int tlballoc(struct pcb_t *proc, uint32_t size, uint32_t reg_index)
{
  int val, addr;
  /* By default using vmaid = 0 */
  struct tlb_cache *tlb_mm = proc->tlb;
  if(size > proc->tlb->count){
	printf("Lack of memory region \n");
	return -1;
  }

  for(int i = 0; i < size; i++){
	struct tlb_node *node = tlb_mm->freelistTail;
	proc->tlb->freelistTail = proc->tlb->freelistTail->prev;
	proc->tlb->freelistTail->next = NULL;

	//Khoi tao cho vung nho duoc cap nhat
	node->next = node->prev = NULL;
	node->reg_index = reg_index;
	node->writedflag = 0;

	//Gan vung nho vua duoc cap nhat vao cuoi
	if(proc->tlb->tlb_head == NULL){
		proc->tlb->tlb_head = proc->tlb->tlb_tail = node;
		proc->tlb->count --;
	}else{
		proc->tlb->tlb_tail->next = node;
		node->prev = proc->tlb->tlb_tail;
		proc->tlb->tlb_tail = node;
		proc->tlb->count --;
	}
  }
  /* TODO update TLB CACHED frame num of the new allocated page(s)*/
  /* by using tlb_cache_read()/tlb_cache_write()*/
  
  val = __alloc(proc, 0, reg_index, size, &addr);
  return val;
}

/*pgfree - CPU TLB-based free a region memory
 *@proc: Process executing the instruction
 *@size: allocated size 
 *@reg_index: memory region ID (used to identify variable in symbole table)
 */
int tlbfree_data(struct pcb_t *proc, uint32_t reg_index)
{
  struct tlb_node *ptr = proc->tlb->tlb_head;
  if(ptr == NULL){
	printf("Invalid memory region\n");
	return -1;
  }

  while(ptr != NULL){
	if(ptr->reg_index == reg_index){
		struct tlb_node *node = ptr;
		if(proc->tlb->tlb_head == ptr) {
			if(proc->tlb->tlb_head == proc->tlb->tlb_tail){
				proc->tlb->tlb_head = proc->tlb->tlb_tail = NULL;
			}else{
				proc->tlb->tlb_head = proc->tlb->tlb_head->next;
				proc->tlb->tlb_head->prev = NULL;
			}
		}else if(ptr == proc->tlb->tlb_tail){
			 proc->tlb->tlb_tail = proc->tlb->tlb_tail->prev;
			 proc->tlb->tlb_tail->next = NULL;
		}else{
			node->next->prev = node->prev;
			node->prev->next = node->next;
		}
		ptr = ptr->next;
		node->writedflag = 0;

		//giai phong con tro hien tai
		node->next = node->prev = NULL;

		//Them vung nho duoc gai phong vao freelist
		proc->tlb->freelistTail->next = node;
		node->prev = proc->tlb->freelistTail;
		proc->tlb->freelistTail = node;

		proc->tlb->count ++;
		continue;
	}
	ptr = ptr->next;
 }
 __free(proc, 0 ,reg_index);

  /* TODO update TLB CACHED frame num of freed page(s)*/
  /* by using tlb_cache_rea d()/tlb_cache_write()*/

  return 0;
}


/*tlbread - CPU TLB-based read a region memory
 *@proc: Process executing the instruction
 *@source: index of source register
 *@offset: source address = [source] + [offset]
 *@destination: destination storage
 */
int tlbread(struct pcb_t * proc, uint32_t source,
            uint32_t offset, 	uint32_t destination) 
{
  BYTE data;
  uint32_t frmnum = -1;
  int srcaddr = source + offset;

  //tlb_cache_read(proc->tlb, srcaddr, &frmnum);

  /* TODO retrieve TLB CACHED frame num of accessing page(s)*/
  /* by using tlb_cache_read()/tlb_cache_write()*/
  /* frmnum is return value of tlb_cache_read/write value*/
	
#ifdef IODUMP
  if (tlb_cache_read(proc->tlb, proc->pid, srcaddr, &frmnum) >= 0)
    printf("TLB hit at read region=%d offset=%d pid=%d\n ", 
	         source, offset, proc->pid);
  else
	printf("TLB miss at read region=%d offset=%d pid=%d\n", 
	         source, offset, proc->pid);
#ifdef PAGETBL_DUMP
  print_pgtbl(proc, 0, -1); //print max TBL
#endif
  MEMPHY_dump(proc->mram);
#endif

  int val = __read(proc, 0, source, offset, &data);

  destination = (uint32_t) data;
  tlb_cache_write (proc->tlb, proc->pid, destination, srcaddr);


  /* TODO update TLB CACHED with frame num of recent accessing page(s)*/
  /* by using tlb_cache_read()/tlb_cache_write()*/

  return val;
}

/*tlbwrite - CPU TLB-based write a region memory
 *@proc: Process executing the instruction
 *@data: data to be wrttien into memory
 *@destination: index of destination register
 *@offset: destination address = [destination] + [offset]
 */
int tlbwrite(struct pcb_t * proc, BYTE data,
             uint32_t destination, uint32_t offset)
{
  int val;
  uint32_t frmnum = -1;
  // tlb_cache_write(proc->tlb, proc->pid, destination, data);
  /* TODO retrieve TLB CACHED frame num of accessing page(s))*/
  /* by using tlb_cache_read()/tlb_cache_write()
  frmnum is return value of tlb_cache_read/write value*/

 // tlb_cache_read(proc->tlb, destination + offset, &frmnum);
  
#ifdef IODUMP
  if (tlb_cache_read(proc->tlb, proc->pid, destination + offset, &frmnum) >= 0)
    printf("TLB hit at write region=%d offset=%d value=%d pid=%d\n",
	          destination, offset, data, proc->pid);
	else
    printf("TLB miss at write region=%d offset=%d value=%d pid=%d\n",
            destination, offset, data, proc->pid);
#ifdef PAGETBL_DUMP
  print_pgtbl(proc, 0, -1); //print max TBL
#endif
  MEMPHY_dump(proc->mram);
#endif

  val = __write(proc, 0, destination, offset, data);
  

  /* TODO update TLB CACHED with frame num of recent accessing page(s)*/
  /* by using tlb_cache_read()/tlb_cache_write()*/

  return val;
}

//#endif
