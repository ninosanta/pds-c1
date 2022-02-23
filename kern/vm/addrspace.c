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

//Librerie
#include <types.h>
#include <kern/errno.h>
#include <lib.h>
#include <current.h>
#include <addrspace.h>
#include <vm.h>
#include <proc.h>
#include <pt.h>
#include <coremap.h>
#include <swapfile.h>
#include <spl.h>
#include <cpu.h>
#include <thread.h>
#include <vnode.h>
#include <vfs.h>
#include <uio.h>

#include "vmstats.h"
#include "vm_tlb.h"

/* Under dumbvm, always have 72k of user stack */
/* (this must be > 64K so argument blocks of size ARG_MAX will fit) */
#define DUMBVM_STACKPAGES 18

// Variabili globali
static unsigned int vm_activated = 0;


/************************************************************
 *                                                          *
 * Implementazione delle funzioni                           *
 *                                                          *
 ************************************************************/

/**
 * @brief Initialization function of the Virtual Memory System 
 * 
 */
void vm_bootstrap(void)
{
	// Inizializzazione della CoreMap
	if (coremap_init() != COREMAP_INIT_SUCCESS)
	{
		panic("cannot init vm system. Low memory!\n");
	}

	// Inizializzazione della Page Table
	if (pt_init() != 0)
	{ // deve avere la dimensione della memoria fisica
		panic("cannot init vm system. Low memory!\n");
	}

	// Inizializzazione del file di Swap
	if (swapfile_init(SWAP_SIZE))
	{
		panic("cannot init vm system. Low memory!\n");
	}

	// Inizializzazione della TLB
	if (vmtlb_init())
	{
		panic("cannot init vm system. Low memory!\n");
	}

	vm_activated = 1;

// Inizializzazione parametri utili per le statistiche
	vmstats_init(); 
}

/**
 * @brief 
 * 
 */
static void vm_can_sleep(void)
{
	if (CURCPU_EXISTS())
	{
		/* must not hold spinlocks */
		KASSERT(curcpu->c_spinlocks == 0);

		/* must not be in an interrupt handler */
		KASSERT(curthread->t_in_interrupt == 0);
	}
}

/**
 * @brief Funzione che riempie di zeri una regione di n pagine a partire dall'indirizzo paddr
 * 
 * @param paddr 
 * @param npages 
 */
void as_zero_region(paddr_t paddr, unsigned npages)
{
	bzero((void *)PADDR_TO_KVADDR(paddr), npages * PAGE_SIZE);
}


/**
 * @brief Funzione che gestisce il vm_fault in caso di pagine di codice 
 * 
 * @param as 
 * @param faultaddress 
 * @param vbase 
 * @param vtop 
 * @param pid 
 * @param paddr 
 * @return int 
 */
static int vm_fault_page_replacement_code(struct addrspace *as, vaddr_t faultaddress, vaddr_t vbase, vaddr_t vtop, pid_t pid, paddr_t *paddr)
{
	int indexReplacement,
		ix = -1;
	size_t to_read;
	int result;
	unsigned char flags = 0;

	//Controllo sul range di indirizzi appartenente allo spazio di codice
	if (faultaddress >= vbase && faultaddress < vtop) 
	{
		// Aggiunge un nuovo processo
		as->count_proc++;

		//Se il processo ha già un numero di pagine in memoria > o = al numero consentito (32)
		//allora devo cercare una pagina del processo stesso da rimpiazzare
		if (as->count_proc >= MAX_PROC_PT)
		{
			//Ottengo l'indice e calcolo l'indirizzo di pagina
			indexReplacement = pt_replace_entry(pid);
			*paddr = indexReplacement * PAGE_SIZE;

			//Ricavo l'indirizzo della entry nella tlb da modificare
			ix = pt_getFlagsByIndex(indexReplacement) >> 2;

			//Faccio swapout della pagina selezionata
			// passando vaddr, paddr, pid, flag
			swapfile_swapout(pt_getVaddrByIndex(indexReplacement), *paddr, pt_getPidByIndex(indexReplacement), pt_getFlagsByIndex(indexReplacement));
			
			//Diminuisco il numero di pagine assegnate al processo, dato che prima avevo incrementato
			as->count_proc--;
		}
		else
		{
			//Se no cerco se è presente uno slot libero tramite la coremap
			//E ottengo l'indirizzo fisico
			*paddr = coremap_getppages(1);

			//Se no cerco una pagina appartenente a qualsiasi processo
			//La strategia implementata è FIFO
			if (*paddr == 0x0)
			{
				//Ottengo l'indice della pagina da rimpiazzare e calcolo l'indirizzo fisico
				indexReplacement =  pt_replace_any_entry();
				*paddr = indexReplacement * PAGE_SIZE;
				
				//Ricavo la entry da modificare nella tlb
				ix = pt_getFlagsByIndex(indexReplacement) >> 2;

				//Faccio swapout
				swapfile_swapout(pt_getVaddrByIndex(indexReplacement), indexReplacement * PAGE_SIZE, pt_getPidByIndex(indexReplacement), pt_getFlagsByIndex(indexReplacement));
				
				//Incremento il numero di pagine del processo in memoria
				as->count_proc--;
				
			}
		}
		//Incremento il numero di TLB misses che hanno richiesto un azzeramento della pagina
		as_zero_region(*paddr, 1);
		vmstats_report_pf_zero_increment();
		
		//In base all'indirizzo, calcolo la quantità di codice da leggere
		if (faultaddress == vbase)
		{
			if (as->code_size < PAGE_SIZE - (as->code_offset & ~PAGE_FRAME))
				to_read = as->code_size;
			else
				to_read = PAGE_SIZE - (as->code_offset & ~PAGE_FRAME);
		}
		else if (faultaddress == vtop - PAGE_SIZE)
		{
			to_read = as->code_size - (as->as_npages1 - 1) * PAGE_SIZE;

			if (as->code_offset & ~PAGE_FRAME)
				to_read -= (as->code_offset & ~PAGE_FRAME);
		}
		else
			to_read = PAGE_SIZE;

		result = load_page_from_elf(as->v,
									*paddr + (faultaddress == vbase ? as->code_offset & ~PAGE_FRAME : 0),
									to_read,
									(faultaddress == vbase) ? as->code_offset : (as->code_offset & PAGE_FRAME) + faultaddress - vbase);

		//Incremento il numero di 
		//tlb misses che hanno richiesto che una pagina venisse caricata dal disco
		//tlb misse che hanno richiesto una lettura dell'ELF file
		vmstats_report_pf_disk_increment();
		vmstats_report_pf_elf_increment();

		//Aggiungo una entry nella page table
		//Settando il flag a read-only essendo una pagina di codice
		flags = 0x01; // Read-only

		if (ix != -1)
			flags |= ix << 2;

		result = pt_add_entry(faultaddress, *paddr, pid, flags);
		if (result < 0)
			return EFAULT;
		return 0;
	}
	else
		return EFAULT;
}

/**
 * @brief Funzione che gestisce il vm_faulr in caso di pagine di dati
 * 
 * @param as 
 * @param faultaddress 
 * @param vbase 
 * @param vtop 
 * @param pid 
 * @param paddr 
 * @return int 
 */
static int vm_fault_page_replacement_data(struct addrspace *as, vaddr_t faultaddress, vaddr_t vbase, vaddr_t vtop, pid_t pid, paddr_t *paddr)
{
	int indexReplacement,
		ix = -1;

	int result;
	size_t to_read;
	unsigned char flags = 0;

	if (faultaddress >= vbase && faultaddress < vtop)
	{
		as->count_proc++;
		if (as->count_proc >= MAX_PROC_PT)
		{
			indexReplacement = pt_replace_entry(pid);
			ix = pt_getFlagsByIndex(indexReplacement) >> 2; 
			swapfile_swapout(pt_getVaddrByIndex(indexReplacement), indexReplacement * PAGE_SIZE, pt_getPidByIndex(indexReplacement), pt_getFlagsByIndex(indexReplacement));
			as->count_proc--;
			*paddr = indexReplacement * PAGE_SIZE;
		}
		else
		{
			*paddr = coremap_getppages(1);
			if (*paddr == 0)
			{
				indexReplacement =  pt_replace_any_entry();
				ix = pt_getFlagsByIndex(indexReplacement) >> 2;
				swapfile_swapout(pt_getVaddrByIndex(indexReplacement), indexReplacement * PAGE_SIZE, pt_getPidByIndex(indexReplacement), pt_getFlagsByIndex(indexReplacement));
				as->count_proc--;
				*paddr = indexReplacement * PAGE_SIZE;
			}
		}

		vmstats_report_pf_zero_increment();

		as_zero_region(*paddr, 1);

		if (faultaddress == vbase)
		{
			if (as->data_size < PAGE_SIZE - (as->data_offset & ~PAGE_FRAME))
				to_read = as->data_size;
			else
				to_read = PAGE_SIZE - (as->data_offset & ~PAGE_FRAME);
		}
		else if (faultaddress == vtop - PAGE_SIZE)
		{
			to_read = as->data_size - (as->as_npages2 - 1) * PAGE_SIZE;
			if (as->data_offset & ~PAGE_FRAME)
				to_read -= (as->data_offset & ~PAGE_FRAME);
		}
		else
			to_read = PAGE_SIZE;

		result = load_page_from_elf(as->v, *paddr + (faultaddress == vbase ? as->data_offset & ~PAGE_FRAME : 0),
									to_read,
									faultaddress == vbase ? as->data_offset : (as->data_offset & PAGE_FRAME) + faultaddress - vbase);

		vmstats_report_pf_disk_increment();
		vmstats_report_pf_disk_increment();

		if (ix != -1)
			flags |= ix << 2;
		result = pt_add_entry(faultaddress, *paddr, pid, flags);

		if (result < 0)
			return -1;

		return 0;
	}
	else
		return EFAULT;
}

/**
 * @brief Funzione che gestisce il vm_fault in caso di pagine che risiedono nello stack
 * 
 * @param as 
 * @param faultaddress 
 * @param stackbase 
 * @param stacktop 
 * @param pid 
 * @param paddr 
 * @return int 
 */
static int vm_fault_page_replacement_stack(struct addrspace *as, vaddr_t faultaddress, vaddr_t stackbase, vaddr_t stacktop, pid_t pid, paddr_t *paddr)
{
	int indexReplacement,
		ix = -1;

	int result;
	unsigned char flags = 0;

	if (faultaddress >= stackbase && faultaddress < stacktop)
	{

		as->count_proc++;
		if (as->count_proc >= MAX_PROC_PT)
		{
			indexReplacement =  pt_replace_any_entry();
			//indexReplacement = pt_replace_entry(pid);
			ix = pt_getFlagsByIndex(indexReplacement) >> 2; // overwrite tlb_index
			swapfile_swapout(pt_getVaddrByIndex(indexReplacement), indexReplacement * PAGE_SIZE, pt_getPidByIndex(indexReplacement), pt_getFlagsByIndex(indexReplacement));
			as->count_proc--;
			*paddr = indexReplacement * PAGE_SIZE;
		}
		else
		{
			*paddr = coremap_getppages(1);
			if (*paddr == 0)
			{
				indexReplacement = pt_replace_entry(pid);
				ix = pt_getFlagsByIndex(indexReplacement) >> 2; // overwrite tlb_index
				swapfile_swapout(pt_getVaddrByIndex(indexReplacement), indexReplacement * PAGE_SIZE, pt_getPidByIndex(indexReplacement), pt_getFlagsByIndex(indexReplacement));
				as->count_proc--;
				*paddr = indexReplacement * PAGE_SIZE;
			}
		}
		as_zero_region(*paddr, 1);

		vmstats_report_pf_disk_increment();

		if (ix != -1)
			flags |= ix << 2;
		result = pt_add_entry(faultaddress, *paddr, pid, flags);
		if (result < 0)
			return -1;

		return 0;
	}
	else
		return EFAULT;
}

/**
 * @brief Fault handling function called by trap code
 * 
 * @param faulttype 
 * @param faultaddress 
 * @return int 
 */
int vm_fault(int faulttype, vaddr_t faultaddress)
{
	vaddr_t vbase1, vtop1, vbase2, vtop2, stackbase, stacktop;
	paddr_t paddr;

	uint32_t ehi, elo;
	struct addrspace *as;
	int status = 0;

	faultaddress &= PAGE_FRAME;
	DEBUG(DB_VM, "dumbvm: fault: 0x%x\n", faultaddress);

	switch (faulttype)
	{
	case VM_FAULT_READONLY:
		/* We always create pages read-write, so we can't get this */
		as_destroy(proc_getas());
		thread_exit();
	case VM_FAULT_READ:
	case VM_FAULT_WRITE:
		vmstats_report_tlb_fault_increment();
		break;
	default:
		return EINVAL;
	}

	if (curproc == NULL)
	{
		/*
		 * No process. This is probably a kernel fault early
		 * in boot. Return EFAULT so as to panic instead of
		 * getting into an infinite faulting loop.
		 */
		return EFAULT;
	}
	as = proc_getas();
	if (as == NULL)
	{
		/*
		 * No address space set up. This is probably also a
		 * kernel fault early in boot.
		 */
		return EFAULT;
	}

	/* Assert that the address space has been set up properly. */
	KASSERT(as->as_vbase1 != 0);
	// KASSERT(as->as_pbase1 != 0);
	KASSERT(as->as_npages1 != 0);
	KASSERT(as->as_vbase2 != 0);
	// KASSERT(as->as_pbase2 != 0);
	KASSERT(as->as_npages2 != 0);
	// KASSERT(as->as_stackpbase != 0);
	KASSERT((as->as_vbase1 & PAGE_FRAME) == as->as_vbase1);
	// KASSERT((as->as_pbase1 & PAGE_FRAME) == as->as_pbase1);
	KASSERT((as->as_vbase2 & PAGE_FRAME) == as->as_vbase2);
	// KASSERT((as->as_pbase2 & PAGE_FRAME) == as->as_pbase2);
	KASSERT((as->as_stackpbase & PAGE_FRAME) == as->as_stackpbase);

	vbase1 = as->as_vbase1;
	vtop1 = vbase1 + (as->as_npages1) * PAGE_SIZE;
	vbase2 = as->as_vbase2;
	vtop2 = vbase2 + (as->as_npages2) * PAGE_SIZE;
	stackbase = USERSTACK - DUMBVM_STACKPAGES * PAGE_SIZE;
	stacktop = USERSTACK;

	paddr_t p_temp;
	pid_t pid = curproc->pid;
	unsigned char flags = 0;

	int ix = -1;

	//Controllo che la pagina sia in page table
	if (pt_get_paddr(faultaddress, pid, &p_temp))
	{
		//Se la pagina è stata trovata in memoria
		paddr = p_temp;
		//Incremento il numero di tlb miss di pagine che erano già in memoria
		vmstats_report_tlb_reload_increment();
	}

	//controllo che la pagina sia nel file di swap
	else if (swapfile_swapin(faultaddress, &p_temp, pid, as))
	{
		//Se la pagina è stata trovato nello swap file ed è stato fatto lo swap
		paddr = p_temp;
		flags = pt_getFlagsByIndex(paddr >> 12);
		//Incremento il numero di tilb misses che hanno richiesto una lettura da disco
		vmstats_report_pf_disk_increment();
	}
	else
	{
		//Gestione del page replacement se la pagina deve essere caricata da disco
		//CODICE
		status = vm_fault_page_replacement_code(as, faultaddress, vbase1, vtop1, pid, &paddr);

		if (status == -1)
			return -1;

		else if (status == EFAULT)
		{
			//DATI
			status = vm_fault_page_replacement_data(as, faultaddress, vbase2, vtop2, pid, &paddr);

			if (status == -1)
				return -1;
			else if (status == EFAULT)
			{
				//STACK
				status = vm_fault_page_replacement_stack(as, faultaddress, stackbase, stacktop, pid, &paddr);

				if (status == -1)
					return -1;
				else if (status == EFAULT)
					return EFAULT;
			}
		}
	}

	/* make sure it's page-aligned */
	KASSERT((paddr & PAGE_FRAME) == paddr);

	
    /*****************************************************************
    * virtual page frame number (20 bit) | pid (6 bit) | pad (6 bit)* EHI
    * **********************************************************************************************
    * physical page frame number (20 bit)| no cache bit | dirty bit | valid bit | global bit | pad * ELO
    * **********************************************************************************************/
	ehi = faultaddress | pid << 6;

	elo = paddr | TLBLO_VALID | TLBLO_GLOBAL; //Se non viene utilizzato il TLBLO_GLOBAL il sistema va in crash
	if ((flags & 0x01) != 0x01)	// se non è settato l'ultimo bit allora la pagina è modificabile
		elo |= TLBLO_DIRTY;					  


	//Aggiungo la entry nella tabella
	vmtlb_write(&ix, ehi, elo);
	KASSERT(ix != -1);

	//Setto i flag della entry nella tabella
	pt_setFlagsAtIndex(paddr >> 12, ix << 2);

	return 0;
}

// /* TLB shootdown handling called from interprocessor_interrupt */
void vm_tlbshootdown(const struct tlbshootdown *ts)
{
	(void)ts;
	panic("dumbvm tried to do tlb shootdown?!\n");
}

/* Allocate kernel heap pages (called by kmalloc/kfree) */
vaddr_t alloc_kpages(unsigned npages)
{
	paddr_t pa;

	vm_can_sleep(); // dumbvm_can_sleep(); Rinominata in vm_can_sleep() per distinguerla dalla dumbvm
	
	//Cerco npages libere nella coremap
	pa = coremap_getppages(npages);
	if (!pa)
		return 0;

	return PADDR_TO_KVADDR(pa);
}

/* Free kernel heap pages (called by kmalloc/kfree) */
void free_kpages(vaddr_t addr)
{
	if (coremap_isTableActive())
	{
		coremap_freepages(addr - MIPS_KSEG0);
	}
}

/*
 * Note! If OPT_DUMBVM is set, as is the case until you start the VM
 * assignment, this file is not compiled or linked or in any way
 * used. The cheesy hack versions in dumbvm.c are used instead.
 */

/**
 * @brief Alloca l'addresspace e inizializza le sue componenti
 * 
 * @return struct addrspace* 
 */
struct addrspace *
as_create(void)
{
	struct addrspace *as;

	as = kmalloc(sizeof(struct addrspace));
	if (as == NULL)
		return NULL;

#if OPT_PAGING

	// Inizializzazione della variabile di tipo addrspace
	as->as_vbase1 = 0;
	as->as_npages1 = 0;
	as->as_vbase2 = 0;
	as->as_npages2 = 0;
	as->as_stackpbase = 0;
	as->count_proc = 0;
	as->v = NULL;

#endif

	return as;
}

/**
 * @brief Copia un addresspace in un nuovo addresspace
 *        Non necessario per lo scopo del progetto
 * 
 * @param old 
 * @param ret 
 * @return int 
 */
int as_copy(struct addrspace *old, struct addrspace **ret)
{
	struct addrspace *newas;

#if OPT_PAGING

	KASSERT(old != NULL);
	KASSERT(old->as_npages1 != 0);
	KASSERT(old->as_npages2 != 0);
	KASSERT(old->as_vbase1 != 0);
	KASSERT(old->as_vbase2 != 0);
	KASSERT(old->as_stackpbase != 0);

#endif

	newas = as_create();
	if (newas == NULL)
	{
		return ENOMEM;
	}

	(void)old;

	*ret = newas;
	return 0;
}

/**
 * @brief Rimuove tutte le entries della page table del processo che si sta distruggendo
 *        Chiude l'ELF file e finalmente libera lo spazio dell'addresspace
 * @param as 
 */
void as_destroy(struct addrspace *as)
{
	vm_can_sleep();

#if OPT_PAGING	
	//Pulisce la page table dalle pagine del processo
	pt_remove_entries(curproc->pid);
	//Chiude il file ELF
	vfs_close(as->v);
#endif

	kfree(as);
}

/**
 * @brief Attiva l'addresspace del processo corrente, invalidando le entry della talb di altri processi
 * 
 */
void as_activate(void)
{
	int i, spl;
	struct addrspace *as;
	uint32_t ehi, elo;
	pid_t pid = curproc->pid;
	char full_inv = 1;


	as = proc_getas();
	if (as == NULL)
	{
		return;
	}
#if OPT_PAGING
	/* Disable interrupts on this CPU while frobbing the TLB. */
	spl = splhigh();

	//Cerca nella tlb le pagine da invalidare
	//Se tutte le pagine sono state invalidate ( full_inv è ancora a 1 )
	//allora incremento la statistica
	for (i = 0; i < NUM_TLB; i++)
	{
		tlb_read(&ehi, &elo, i);
		if (((ehi & TLBHI_PID) >> 6) == (unsigned int)pid)
		{
			full_inv = 0;
			continue;
		}
		else{
			vmtlb_clean(i); // tlb_write(TLBHI_INVALID(i), TLBLO_INVALID(), i); 
		}
	}

	if (full_inv){
		vmstats_report_tlb_invalidation_increment();
	}

	splx(spl);
#endif
}


void as_deactivate(void)
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

/**
 * @brief Definisce una regione dell'addresspace settando l'indirizzo base, il numero di pagine, la dimensione e il vnode
 * 
 */
int as_define_region(struct addrspace *as, vaddr_t vaddr, size_t memsize, struct vnode *v,
					 int readable, int writeable, int executable,
#if OPT_PAGING
					 off_t offset
#endif
)
{
	KASSERT(as != NULL);

	size_t npages;
	size_t memsize_old = memsize;

	vm_can_sleep();

	/* Align the region. First, the base... */
	memsize += vaddr & ~(vaddr_t)PAGE_FRAME;
	vaddr &= PAGE_FRAME;

	/* ...and now the length. */
	memsize = (memsize + PAGE_SIZE - 1) & PAGE_FRAME;

	npages = memsize / PAGE_SIZE;

	/* We don't use these - all pages are read-write */
	(void)readable;
	(void)writeable;
	(void)executable;

	if (as->as_vbase1 == 0)
	{
		as->as_vbase1 = vaddr;
		as->as_npages1 = npages;
		as->code_offset = offset;
		as->code_size = memsize_old;
		as->v = v;
		return 0;
	}

	if (as->as_vbase2 == 0)
	{
		as->as_vbase2 = vaddr;
		as->as_npages2 = npages;
		as->data_offset = offset;
		as->data_size = memsize_old;
		as->v = v;
		return 0;
	}

	/*
	 * Support for more than two regions is not available.
	 */
	kprintf("dumbvm: Warning: too many regions\n");
	return EFAULT;
}




int as_prepare_load(struct addrspace *as)
{
	// Non ci serve perchè stiamo usando la inverted page table
	/*
	 * Write this.
	 */
	(void)as;
	return 0;
}

int as_complete_load(struct addrspace *as)
{
	/*
	 * Write this.
	 */

	(void)as;
	return 0;
}

int as_define_stack(struct addrspace *as, vaddr_t *stackptr)
{
	/*
	 * Write this.
	 */

	(void)as;

	/* Initial user-level stack pointer */
	*stackptr = USERSTACK;

	return 0;
}