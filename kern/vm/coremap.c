// coremap.c: keep track of free physical frames
#include <types.h>
#include <kern/errno.h>
#include <lib.h>
#include <vm.h>
#include "coremap.h"


//andare a vedere in OS161 MEMORY slide --> pagina 19 



/*si può utilizzare una struttura coremap_entry per tenere traccia delle informazioni sulla pagina fisica
coremap sarà un array di coremap_entry.
Deve indicare se il frame è libero, se è occupato, indica quanti frame contigui per la stessa locazione


coremap entry
coremap ( tipo bitmap ) 

*/

// /* Initialization function of the CoreMap  */
int coremap_init( size_t ram_size, vaddr_t first_free_addr) {
//     //allocata tra firstaddr(0) e freeaddr --> questa parte in realtà si può non mappare perchè non sono frame che si possono liberare 
//     //tra 0 e freeaddr index --> pagine fixed ( di cui non si può fare swap out )  
//     //tra freeaddr e lastaddr(ram_size) --> pagine free
 }

//Deve trovare un segmento libero 
void coremap_find_free (void){
    //cercare un frame libero
    //se libero chiama la funzione che lo occupi
    //se non c'è nessun frame libero 
    //chiama la funzione che faccia swap out di qualcosa
}

//Deve trovare N segmenti liberi contigui
void coremap_find_nfree(void/*int num_seg*/){
    //cerca n frame contigui liberi 
    //se li trova, chiama la funzione che li occupi
    //se non li trova
    //chiama la funzione che li trovi
}







