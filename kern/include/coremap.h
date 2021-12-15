#ifndef _COREMAP_H_
#define _COREMAP_H_

#define COREMAP_RETURN_SUCCESS 0

int coremap_init(void);
int coremap_isTableActive(void);
paddr_t coremap_getppages(unsigned long npages);
int coremap_freepages(paddr_t addr);

#endif


// Da valutare
//Deve trovare un segmento libero 
//void coremap_find_free (void); 

//Deve trovare N segmenti liberi contigui
//void coremap_find_nfree(void/*int num_seg*/); 

//i tipi di ritorno e i parametri sono da cambiare come serve