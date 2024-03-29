/*
 * Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2008, 2009
 *	The President and Fellows of Harvard College.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE UNIVERSITY AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE UNIVERSITY OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef _ADDRSPACE_H_
#define _ADDRSPACE_H_

/*
 * Address space structure and operations.
 */

#include <types.h>
#include <elf.h>
#include <current.h>
#include "opt-paging.h"

struct vnode;

/*
 * Address space - data structure associated with the virtual memory
 * space of a process.
 *
 */

struct addrspace {
#if OPT_PAGING
        vaddr_t as_vbase1;  /* base virtual address of code segment */
        size_t as_npages1;  /* size (in pages) of code segment */
        vaddr_t as_vbase2;  /* base virtual address of data segment */
        size_t as_npages2;  /* size (in pages) of data segment */
        paddr_t as_stackpbase;  /* base physical address of stack */

        // Addition data
        off_t code_offset; // code offset within elf file
        uint32_t code_size;
        off_t data_offset; // data offset within elf file
        uint32_t data_size;
        struct vnode * v; // file descriptor

        int count_proc; // COntatore pagine di questo processo caricato in page table

#else
        /* Put stuff here for your VM system */
       
#endif
};

/*
 * Functions in addrspace.c:
 *
 *    as_create - create a new empty address space. You need to make
 *                sure this gets called in all the right places. You
 *                may find you want to change the argument list. May
 *                return NULL on out-of-memory error.
 *
 *    as_copy   - create a new address space that is an exact copy of
 *                an old one. Probably calls as_create to get a new
 *                empty address space and fill it in, but that's up to
 *                you.
 *
 *    as_activate - make curproc's address space the one currently
 *                "seen" by the processor.
 *
 *    as_deactivate - unload curproc's address space so it isn't
 *                currently "seen" by the processor. This is used to
 *                avoid potentially "seeing" it while it's being
 *                destroyed.
 *
 *    as_destroy - dispose of an address space. You may need to change
 *                the way this works if implementing user-level threads.
 *
 *    as_define_region - set up a region of memory within the address
 *                space.
 *
 *    as_prepare_load - this is called before actually loading from an
 *                executable into the address space.
 *
 *    as_complete_load - this is called when loading from an executable
 *                is complete.
 *
 *    as_define_stack - set up the stack region in the address space.
 *                (Normally called *after* as_complete_load().) Hands
 *                back the initial stack pointer for the new process.
 *
 * Note that when using dumbvm, addrspace.c is not used and these
 * functions are found in dumbvm.c.
 */

struct addrspace *as_create(void);
int               as_copy(struct addrspace *src, struct addrspace **ret);
void              as_activate(void);
void              as_deactivate(void);
void              as_destroy(struct addrspace *);

int               as_define_region(struct addrspace *as,
                                   vaddr_t vaddr, size_t memsize,
                                   struct vnode *v,
                                   int readable,
                                   int writeable,
                                   int executable,
                                   #if OPT_PAGING
                                   off_t offset
                                   #endif
                                   );
int               as_prepare_load(struct addrspace *as);
int               as_complete_load(struct addrspace *as);
int               as_define_stack(struct addrspace *as, vaddr_t *initstackptr);
void as_zero_region(paddr_t paddr, unsigned npages); 

/*
 * Functions in loadelf.c
 *    load_elf - load an ELF user program executable into the current
 *               address space. Returns the entry point (initial PC)
 *               in the space pointed to by ENTRYPOINT.
 
*/
int load_elf(struct vnode *v, vaddr_t *entrypoint);
int load_page_from_elf(struct vnode *v, paddr_t dest, size_t len, off_t offset); 



//void vm_bootstrap(void); 



//* TLB shootdown handling called from interprocessor_interrupt /
//void vm_tlbshootdown(const struct tlbshootdown *); 
///* Fault handling function called by trap code /
//int vm_fault(int faulttype, vaddr_t faultaddress);



#endif /* _ADDRSPACE_H_ */
