/*
 * Copyright (C) 2024 pdnguyen of the HCMC University of Technology
 */
/*
 * Source Code License Grant: Authors hereby grants to Licensee 
 * a personal to use and modify the Licensed Source Code for 
 * the sole purpose of studying during attending the course CO2018.
 */
//#ifdef MM_TLB
/*
 * Memory physical based TLB Cache
 * TLB cache module tlb/tlbcache.c
 *
 * TLB cache is physically memory phy
 * supports random access 
 * and runs at high speed
 */


#include "mm.h"
#include <stdlib.h>
#include <stdio.h>



#define init_tlbcache(mp,sz,...) init_memphy(mp, sz, (1, ##__VA_ARGS__))

/*
 *  tlb_cache_read read TLB cache device
 *  @mp: memphy struct
 *  @pid: process id
 *  @pgnum: page number
 *  @value: obtained value
 */
int tlb_cache_read(struct tlb_cache * mcache, uint32_t vmaddr, uint32_t *frnum)
{
   /* TODO: the identify info is mapped to 
    *      cache line by employing:
    *      direct mapped, associated mapping etc.
    */
   if(mcache->tlb_head == NULL){
	printf("Uninitial memory region \n");
	return -1;
   }
   struct tlb_node * ptr = mcache->tlb_head;
   while (ptr != NULL)
   {
      if (ptr->memvm == vmaddr && ptr->writedflag == 1)
      {
         *frnum = ptr->memphy;
         // neu vi tri nao duoc goi se nhay len dau dam bao viec kien truc LRU
         if (ptr != mcache->tlb_head)
         {
            if (ptr == mcache->tlb_tail)
            {
               mcache->tlb_tail = ptr->prev;
               mcache->tlb_tail->next = NULL;
            }else{
	       ptr->next->prev =ptr->prev;
	    }

            ptr->prev->next = ptr->next;

            ptr->prev = NULL;
            ptr->next = mcache->tlb_head;
            mcache->tlb_head->prev = ptr;

            mcache->tlb_head = ptr;
         }

         return 0;
      }
      ptr = ptr->next;
   }
   
   return -1;
}

/*
 *  tlb_cache_write write TLB cache device
 *  @mp: memphy struct
 *  @pid: process id
 *  @pgnum: page number
 *  @value: obtained value
 */
int tlb_cache_write(struct tlb_cache *mcache, uint32_t memphy, uint32_t memvm)
{
   /* TODO: the identify info is mapped to 
    *      cache line by employing:
    *      direct mapped, associated mapping etc.
    */
   if(mcache->tlb_head == NULL){
	printf("Uninitial memory region \n");
	return -1;
   }
   struct tlb_node *ptrcheck = mcache->tlb_head;
   while (ptrcheck != NULL)
   {
      if (ptrcheck->memvm == memvm && ptrcheck->memvm == 1)
      {	 
	 if(ptrcheck == mcache->tlb_head){
		 return 0;
	 }
	 if(mcache->count == 2){
		mcache->tlb_head = mcache->tlb_tail;
		mcache->tlb_head->prev = NULL;
		mcache->tlb_head->next = ptrcheck;
		mcache->tlb_tail = ptrcheck;
		mcache->tlb_tail->next = NULL;
		mcache->tlb_tail->prev = mcache->tlb_head;
	 }
         if (ptrcheck == mcache->tlb_tail)
         {
            mcache->tlb_tail = ptrcheck->prev;
            mcache->tlb_tail->next = NULL;
         }


	         ptrcheck->memphy = memphy;

	         ptrcheck->prev->next = ptrcheck->next;
	      	 ptrcheck->next->prev =ptrcheck->prev;

	         ptrcheck->prev = NULL;
        	 ptrcheck->next = mcache->tlb_head;
        	 mcache->tlb_head->prev = ptrcheck;

        	 mcache->tlb_head = ptrcheck;
	

         return 0;
      }
      ptrcheck = ptrcheck->next;
   }

   struct tlb_node *ptr = mcache->tlb_head;
   while(ptr != NULL){
	if(ptr->writedflag == 0){
		ptr->memphy = memphy;
		ptr->memvm = memvm;
		ptr->writedflag = 1;
		
		if(ptr != mcache->tlb_head){
			if(ptr == mcache->tlb_tail){
				ptr->prev->next = NULL;
				mcache->tlb_tail = ptr->prev;
				ptr->prev = NULL;
			}else{
				//Giai phong node vua duowc ghi
				ptr->prev->next = ptr->next;
				ptr->next->prev = ptr->prev;
				ptr->next = ptr->prev = NULL;
			}
			//Chuyen node vua luu len head
			ptr->next = mcache->tlb_head;
			mcache->tlb_head->prev = ptr;
			mcache->tlb_head = ptr;
		}
		return 0;
	}
	ptr = ptr->next;
   } 

   //Neu khong co vung nho trong thi xoa du lieu
   struct tlb_node *node = mcache->tlb_tail;
   node->memphy = memphy;
   node->memvm = memvm;
   node->writedflag = 1;

   mcache->tlb_tail = mcache->tlb_tail->prev;
   mcache->tlb_tail->next = NULL;
   node->next = node->prev = NULL;

   node->next = mcache->tlb_head;
   mcache->tlb_head->prev = node;
   mcache->tlb_head = node;
   return -1;
}

/*
 *  TLBMEMPHY_read natively supports MEMPHY device interfaces
 *  @mp: memphy struct
 *  @addr: address
 *  @value: obtained value
 */
int TLBMEMPHY_read(struct memphy_struct * mp, int addr, BYTE *value)
{
   if (mp == NULL)
     return -1;

   /* TLB cached is random access by native */
   *value = mp->storage[addr];

   return 0;
}


/*
 *  TLBMEMPHY_write natively supports MEMPHY device interfaces
 *  @mp: memphy struct
 *  @addr: address
 *  @data: written data
 */
int TLBMEMPHY_write(struct memphy_struct * mp, int addr, BYTE data)
{
   if (mp == NULL)
     return -1;

   /* TLB cached is random access by native */
   mp->storage[addr] = data;

   return 0;
}

/*
 *  TLBMEMPHY_format natively supports MEMPHY device interfaces
 *  @mp: memphy struct
 */


int TLBMEMPHY_dump(struct memphy_struct * mp)
{
   /*TODO dump memphy contnt mp->storage 
    *     for tracing the memory content
    */

   return 0;
}


/*
 *  Init TLBMEMPHY struct
 */
int init_tlbmemphy(struct tlb_cache *mcache, int max_size)
{
   mcache->tlb_head = mcache->tlb_tail = NULL;
   mcache->maxsize = max_size;
   mcache->count = 0;
   mcache->freelistHead = mcache->freelistTail = NULL;
   for(int i = 0;i < max_size;i++){
	struct tlb_node *node = malloc(sizeof(struct tlb_node));
	node->next = node->prev = NULL;
	if(mcache->freelistHead == NULL){
		mcache->freelistHead = mcache->freelistTail = node;
		mcache->count ++;
	}else{
		mcache->freelistTail->next = node;
		node->prev = mcache->freelistTail;
		mcache->freelistTail = node;
		mcache->count ++;
	}
	node->id = i;
   }

   return 0;
}

//#endif
