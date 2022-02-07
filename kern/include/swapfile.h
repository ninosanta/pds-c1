#ifndef _SWAPFILE_H_
#define _SWAPFILE_H_

#include <types.h>
#include <spinlock.h>
#include <addrspace.h>
#define SWAPMAP_INIT_SUCCESS 0
#define SWAPMAP_SUCCESS 0
#define SWAPMAP_REJECT 1


typedef struct  {
    vaddr_t v_pages; 
    pid_t pid; 
    unsigned char flags;
}swapfile;


int swapfile_init(int lenght);
int swapfile_swapin(vaddr_t vaddr, paddr_t *paddr, pid_t pid, struct addrspace *as);
int swapfile_swapout(vaddr_t vaddr, paddr_t paddr, pid_t pid, unsigned char flags);

#endif