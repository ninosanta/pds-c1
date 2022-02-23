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

#ifndef _VM_TLB_H_
#define _VM_TLB_H_

#include <types.h>
#include <kern/errno.h>
#include <mips/tlb.h>

int vmtlb_init(void);
void vmtlb_write(int* index, uint32_t ehi, uint32_t elo);
void vmtlb_clean(int index);


#endif