/**
 * @file pt.h
 * @author your name (you@domain.com)
 * @brief code for managing the inverted page table
 * @version 0.1
 * @date 2022-02-23
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <types.h>
#include <lib.h>

typedef struct pt_fifo_node pt_FIFO;

struct ipt_t{
        pid_t pid;
        vaddr_t vaddr;
        bool invalid;
        unsigned char flags;
        unsigned int counter;
};


int pt_init (void);

int pt_add_entry( vaddr_t vaddr , paddr_t paddr, pid_t pid, /* bool readonly,  */unsigned char flag );

int pt_replace_entry( pid_t pid); 

int pt_replace_any_entry(void);

void pt_remove_entries(pid_t pid);

unsigned int pt_get_paddr ( vaddr_t vaddr, pid_t pid , paddr_t* paddr);

void pt_remove_entry(int index_replace);

void pt_destroy ( void );

vaddr_t pt_getVaddrByIndex(int index); 

unsigned char pt_getFlagsByIndex(int index); 

pid_t pt_getPidByIndex(int index);

void pt_setFlagsAtIndex(int index, unsigned char val);




