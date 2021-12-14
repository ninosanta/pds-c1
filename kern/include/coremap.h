#include <types.h>
#include <vm.h>



int coremap_init(size_t ram_size, vaddr_t first_free_addr);

//Deve trovare un segmento libero 
void coremap_find_free (void); 

//Deve trovare N segmenti liberi contigui
void coremap_find_nfree(void/*int num_seg*/); 

//i tipi di ritorno e i parametri sono da cambiare come serve