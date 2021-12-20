#ifndef _VM_TLB_H_
#define _VM_TLB_H_

#include <types.h>
#include <kern/errno.h>

/**
 * @file vm_tlb.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2021-12-19
 * 
 * @copyright Copyright (c) 2021
 * 
 */

int vmtlb_init(void);
void vmtlb_write(int* index, uint32_t ehi, uint32_t elo);
void vmtlb_clean(int index);



/*ENTRY DELLA TLB
struct tlb_t {
        vaddr_t virtual_page_num; 
        paddr_t physical_page_num; 
        //bool global;//UNUSED //1 ignora i bit del pid
        bool valid;  //1 indica come valida la entry
        bool dirty;  //0 (read only) 1( read write)
        //bool nocache; //UNUSED
        //pid_t pid; //UNUSED //address space id che, se indica che la entry 
        //appartiene al processo corrente, permette ad essa 
        //di rimanere nella tlb
};
*/
#endif