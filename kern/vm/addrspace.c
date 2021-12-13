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

#include <types.h>
#include <kern/errno.h>
#include <lib.h>
#include <addrspace.h>
#include <vm.h>
#include <proc.h>



void vm_bootstrap(void){
    // int val_ret_coremap; 
    // paddr_t val_lastaddr , val_firstaddr, val_firstfree; 
    // int dim_free_ram; 
    // bool vm_bootstrap_done; //dovrebbe essere una variabile globale

    // //inizializza tutte le strutture necessarie 

    // //chiama ram_get_size per ottenere lastaddr e firstaddr(perchè poi dopo saranno diabilitate)
    // val_lastaddr = ram_getsize(); 
    // //alloca coremap 
    // val_firstaddr = 0 ; //in realtà non serve
    // val_firstfree = ram_getfirstfree(); 

    // dim_free_ram = val_lastaddr - val_firstfree; 

    // //inizializza la coremap 
    // coremap_init( dim_free_ram ); 
    // vm_bootstrap_done = true; 


	
	//altre inizializzazione da fare prima FORSE 
	// if(pagetable_init(ram_getsize()/PAGE_SIZE)){
	// 	panic("cannot init vm system. Low memory!\n");
  	// }
  
  	// if(swapfile_init(SWAP_SIZE)){
	//   	panic("cannot init vm system. Low memory!\n");
  	// }
	
	// if (tlb_map_init()){
	// 	panic("cannot init vm system. Low memory!\n");
	// }
	
	//Inizializzazione della CoreMap
	if (coremap_init(ram_getsize())){
		panic("cannot init vm system. Low memory!\n");
	}


	// tlb_f = tlb_ff = tlb_fr = tlb_r = tlb_i = pf_z = 0;
	// pf_d = pf_e = 0;
}

/* Fault handling function called by trap code */
int vm_fault(int faulttype, vaddr_t faultaddress){
//gestore dell'eccezione di MISS
     //TLB miss handler
     //if spazio libero 
     //if not
        //round robin replacement
    (int)faultaddress++; 
    //deve anche controllare che tutte le entry si riferiscano al processo corrente (?) 
    //non capisco se lo deve controllare quando bisogna aggiungere una nuova entry, se la entry si riferisce ad un nuovo processo, allora invalido tutte le altre che non si riferiscono a quello ???
    return faulttype;
} 

/* TLB shootdown handling called from interprocessor_interrupt */
 void
vm_tlbshootdown(const struct tlbshootdown *ts)
{
	(void)ts;
	panic("dumbvm tried to do tlb shootdown?!\n");
}

/*
 * Note! If OPT_DUMBVM is set, as is the case until you start the VM
 * assignment, this file is not compiled or linked or in any way
 * used. The cheesy hack versions in dumbvm.c are used instead.
 */

struct addrspace *
as_create(void)
{
	struct addrspace *as;

	as = kmalloc(sizeof(struct addrspace));
	if (as == NULL) {
		return NULL;
	}

	/*
	 * Initialize as needed.
	 */


	//kmalloc inizializza address space
	//page_alloc alloca una pagina fisica --> che tecnicamente deve essere usata per la page table del processo, ma non dovrebbe servire dato che usiamo un'inverted page table
	//salva in struct addrspace l'indirizzo della page table (?)
	//as_define_region per salvare nell'addresspace le regioni userdefined

	return as;
}

int
as_copy(struct addrspace *old, struct addrspace **ret)
{
	struct addrspace *newas;

	newas = as_create();
	if (newas==NULL) {
		return ENOMEM;
	}

	/*
	 * Write this.
	 */

	(void)old;

	*ret = newas;
	return 0;
}

void
as_destroy(struct addrspace *as)
{
	/*
	 * Clean up as needed.
	 */

	kfree(as);
}

void
as_activate(void)
{
	struct addrspace *as;

	as = proc_getas();
	if (as == NULL) {
		/*
		 * Kernel thread without an address space; leave the
		 * prior address space in place.
		 */
		return;
	}

	/*
	 * Write this.
	 */
}

void
as_deactivate(void)
{
	/*
	 * Write this. For many designs it won't need to actually do
	 * anything. See proc.c for an explanation of why it (might)
	 * be needed.
	 */
}

/*
 * Set up a segment at virtual address VADDR of size MEMSIZE. The
 * segment in memory extends from VADDR up to (but not including)
 * VADDR+MEMSIZE.
 *
 * The READABLE, WRITEABLE, and EXECUTABLE flags are set if read,
 * write, or execute permission should be set on the segment. At the
 * moment, these are ignored. When you write the VM system, you may
 * want to implement them.
 */
int
as_define_region(struct addrspace *as, vaddr_t vaddr, size_t memsize,
		 int readable, int writeable, int executable)
{
	/*
	 * Write this.
	 */

	(void)as;
	(void)vaddr;
	(void)memsize;
	(void)readable;
	(void)writeable;
	(void)executable;
	return ENOSYS;
}

int
as_prepare_load(struct addrspace *as)
{
	/*
	 * Write this.
	 */

	(void)as;
	return 0;
}

int
as_complete_load(struct addrspace *as)
{
	/*
	 * Write this.
	 */

	(void)as;
	return 0;
}

int
as_define_stack(struct addrspace *as, vaddr_t *stackptr)
{
	/*
	 * Write this.
	 */

	(void)as;

	/* Initial user-level stack pointer */
	*stackptr = USERSTACK;

	return 0;
}




