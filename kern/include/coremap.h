#ifndef _COREMAP_H_
#define _COREMAP_H_

int coremap_init(void); // Eventualmente aggiungere questi parametri: size_t ram_size, vaddr_t first_free_addr
int coremap_isTableActive(void);
paddr_t coremap_getppages(unsigned long npages);
int freepages(paddr_t addr, unsigned long npages);

#endif


// Da valutare
//Deve trovare un segmento libero 
//void coremap_find_free (void); 

//Deve trovare N segmenti liberi contigui
//void coremap_find_nfree(void/*int num_seg*/); 

//i tipi di ritorno e i parametri sono da cambiare come serve