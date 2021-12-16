// segments.c: code for tacking and manipulating segments

#include <segments.h>
#include <types.h>



void page_alloc(void) {   //Deve allocare un'unica pagina fisica
 
    //acquisice lock della coremap ( non so se lo deve fare qua) 

    //guarda nella coremap se ci sono pagine FREE 
    /*if (coremap_find_free()) {
         //se c'è alloca la pagina
         coremap_get_entry(); //aggiorna la coremap
    }
    else {
           //se tutte occupate
           //fa swap con una di quelle vecchie (FIFO)
    //aggionra la coremap
    }*/
    //rilascia lock della coremap  

}

void page_nalloc(void) {
    //Deve allocare n pagine fisiche contigue
    //acquisice lock della coremap ( non so se lo deve fare qua) 
    //guarda nella coremap se ci sono n pagine FREE 
    //se ci sono le alloca 
    //aggiorna la coremap
    //se non abbastanza deve torvarne per fare swap out
    //fa swap con una di quelle vecchie (FIFO)
    //aggionra la coremap 
    //rilascia lock della coremap 

}

void page_free(void) {

} 