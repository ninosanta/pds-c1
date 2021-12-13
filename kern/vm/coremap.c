// coremap.c: keep track of free physical frames
#include <types.h>
#include <kern/errno.h>
#include <lib.h>
#include <vm.h>
#include "coremap.h"



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


int coremap_init( size_t ram_size) {
//     //allocata tra firstaddr e freeaddr
//     //tra 0 e freeaddr index --> pagine fixed
//     //tra freeaddr e lastaddr --> pagne free
 }


int vm_is_active(void){
    //si può usare per verificare chela mmeoria virtuale sia stata bootstrappata
    //si fa una variabile globale che viene settate alla fine di Vm_bootstrap 
    if( 1 ) //se non è stata bootrappata
        return 0;
    else  // se è stata bootstrappata 
        return 1 ; 

}




/* Allocate kernel heap pages (called by kmalloc/kfree) */
vaddr_t alloc_kpages(unsigned npages){
    npages++; 
    if ( vm_is_active() ){
        //routine post vm_bootstrap
        //prevede di utilizare page_nalloc

    }
    else {
        //routine pre vm_bootstrap
        //prevede di utilizzare getppages ( penso si possa recuperare il pezzo da dumbvm )

    }
    return 2 ; 
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
    (int)addr++; 
}

/* TLB shootdown handling called from interprocessor_interrupt */
 void
vm_tlbshootdown(const struct tlbshootdown *ts)
{
	(void)ts;
	panic("dumbvm tried to do tlb shootdown?!\n");
}
