// coremap.c: keep track of free physical frames
#include "coremap.h"
#include <types.h>
#include <kern/errno.h>
#include <lib.h>
#include <spl.h>
#include <cpu.h>
#include <spinlock.h>
#include <proc.h>
#include <current.h>
#include <mips/tlb.h>
#include <addrspace.h>
#include <vm.h>



/*si può utilizzare una struttura coremap_entry per tenere traccia delle informazioni sulla pagina fisica
coremap sarà un array di coremap_entry

coremap entry - where is mapped (virtual address --> for swapping) 
              - page status ( free, clean , fixed ...) 

*/
/*
coremap entry
coremap ( tipo bitmap ) 
*/

// /* Initialization function of the Virtual Memory System  */
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
}

// int coremap_init( paddr_t ram_size) {
//     //allocata tra firstaddr e freeaddr
//     //tra 0 e freeaddr index --> pagine fixed
//     //tra freeaddr e lastaddr --> pagne free
// }


int vm_is_active(){
    //si può usare per verificare chela mmeoria virtuale sia stata bootstrappata
    //si fa una variabile globale che viene settate alla fine di Vm_bootstrap 
    if( 1 ) //se non è stata bootrappata
        return 0;
    else  // se è stata bootstrappata 
        return 1 

}

/* Fault handling function called by trap code */
int vm_fault(int faulttype, vaddr_t faultaddress){
//gestore dell'eccezione di MISS
     //TLB miss handler
     //if spazio libero 
     //if not
        //round robin replacement

    //deve anche controllare che tutte le entry si riferiscano al processo corrente (?) 
    //non capisco se lo deve controllare quando bisogna aggiungere una nuova entry, se la entry si riferisce ad un nuovo processo, allora invalido tutte le altre che non si riferiscono a quello ???
} 


/* Allocate kernel heap pages (called by kmalloc/kfree) */
vaddr_t alloc_kpages(unsigned npages){
    if ( vm_is_active() ){
        //routine post vm_bootstrap
        //prevede di utilizare page_nalloc

    }
    else {
        //routine pre vm_bootstrap
        //prevede di utilizzare getppages ( penso si possa recuperare il pezzo da dumbvm )

    }
}

/* Free kernel heap pages (called by kmalloc/kfree) */
void free_kpages(vaddr_t addr){
     if ( vm_is_active() ){
        //routine post vm_bootstrap
        //prevede di utilizare page_free --> ricordiamo che si deve tenere conto di quante sono le pagine contigue allocate quando si allocano in page_nalloc

    }
    else {
        //routine pre vm_bootstrap
        //prevede di utilizzare freeppages( penso si possa recuperare il pezzo da dumbvm )

    }
}

/* TLB shootdown handling called from interprocessor_interrupt */
 void
vm_tlbshootdown(const struct tlbshootdown *ts)
{
	(void)ts;
	panic("dumbvm tried to do tlb shootdown?!\n");
}
