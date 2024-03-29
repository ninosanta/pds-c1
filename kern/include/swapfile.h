/**
 * @file swapfile.h
 * @author your name (you@domain.com)
 * @brief code for managing and manipulating the swapfile
 * @version 0.1
 * @date 2022-02-23
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef _SWAPFILE_H_
#define _SWAPFILE_H_

#include <types.h>
#include <spinlock.h>
#include <addrspace.h>
#define SWAPMAP_INIT_SUCCESS 0
#define SWAPMAP_SUCCESS 1
#define SWAPMAP_REJECT 0


typedef struct  {
    vaddr_t v_pages; 
    pid_t pid; 
    unsigned char flags;
}swapfile;


int swapfile_init(long lenght);
int swapfile_swapin(vaddr_t vaddr, paddr_t *paddr, pid_t pid, struct addrspace *as);
int swapfile_swapout(vaddr_t vaddr, paddr_t paddr, pid_t pid, unsigned char flags);

#endif