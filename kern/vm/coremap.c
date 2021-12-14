/**
 * @file coremap.c
 * @author your name (you@domain.com)
 * @brief Tenere traccia dei frame fisici.
 * @version 0.1
 * @date 2021-12-14
 * 
 * @copyright Copyright (c) 2021
 *
 */

// Librerie
#include <types.h>
#include <kern/errno.h>
#include <lib.h>
#include <vm.h>
#include <spinlock.h>

#include "coremap.h"

/************************************************************
 *                                                          *
 * Osservazioni o note utili:                               *
 *  - Andare a vedere in OS161 MEMORY slide --> pagina 19   *
 *                                                          *
 ************************************************************/

/*si può utilizzare una struttura coremap_entry per tenere traccia delle informazioni sulla pagina fisica
coremap sarà un array di coremap_entry.
Deve indicare se il frame è libero, se è occupato, indica quanti frame contigui per la stessa locazione


coremap entry
coremap ( tipo bitmap ) 

*/

/**
 * @brief La primo passo del sistema di memoria virtuale OS161 è la gestione delle pagine fisiche.
 * In generale, possiamo impacchettare le informazioni di una pagina fisica in una struttura (struct coremap_entry) e usalra per
 * rappresentare una pagina fisica. Usiamo un array di array struct coremap_entry per mantenere tutte le informazioni sulle pagine
 * fisiche
 * 
 */

// Variabili globali
static struct spinlock freemem_lock = SPINLOCK_INITIALIZER; // Gestione in mutua esclusione
static struct spinlock stealmem_lock = SPINLOCK_INITIALIZER;

static unsigned int nRamFrames = 0; // Vettore dinamico della memoria Ram assegnata al Boot (dipende da sys161.conf)
static unsigned char* freeRamFrames = NULL; // Vettore di marcaggi delle locazioni oppupate e libere
static unsigned long* allocSize = NULL; // Dimensione allocata alle varie pagine
static int allocTableActive = 0;

/************************************************************
 *                                                          *
 * Implementazione delle funzioni                           *
 *                                                          *
 ************************************************************/
/**
 * @brief 
 * 
 * @param ram_size 
 * @param first_free_addr 
 * @return int 
 * 
 * Comments: allocata tra firstaddr(0) e freeaddr --> questa parte in realtà si può non mappare perchè non sono frame che si possono liberare
 *          tra 0 e freeaddr index --> pagine fixed (di cui non si può fare swap out) tra freeaddr e lastaddr(ram_size) --> pagine free
 *          
 */
int coremap_init(void){
    int i = 0;

    nRamFrames = ((int)ram_getsize() / PAGE_SIZE);

    // Allocazione dei vettori freeRamFrame e allocSize
    if(!(freeRamFrames = kmalloc(sizeof(unsigned char) * nRamFrames)) || 
        !(allocSize = kmalloc(sizeof(unsigned long) * nRamFrames))) {
        freeRamFrames = allocSize = NULL;

        return ENOMEM;
    }

    // Inizializzazione dei vettori freeRamFrame e allocSize
    for(i = 0; i < nRamFrames; i++){
        freeRamFrames[i] = (unsigned char)0;
        allocSize[i] = (unsigned long)0;
    }

    spinlock_acquire(&freemem_lock);
    allocTableActive = 1; // Tabella allocata correttamente
    spinlock_release(&freemem_lock);

    return 0;
}

/**
 * @brief Analizza in mutua esclusione la variabile denominata active
 * 
 * @return int 
 */
int coremap_isTableActive(void) {
    int active;

    spinlock_acquire(&freemem_lock);
    active = allocTableActive;
    spinlock_release(&freemem_lock);

    return active;
}

/**
 * @brief 
 * 
 * @param npages 
 * @return paddr_t 
 */
static paddr_t coremap_getfreeppages(unsigned long npages){
    paddr_t addr;
    long i, 
        first, 
        found,
        np = (long)npages;

    if(!coremap_isTableActive())
        return 0;
    
    spinlock_acquire(&freemem_lock);

    // Ricerca lineare degli intervalli liberi
    for(i = 0, first = found = -1; i < nRamFrames; i++){
        if(freeRamFrames[i]) {
            if(!i || !freeRamFrames[i-1])
                first = i; // Imposta il primo intervallo libero
            if(i-first+1 >= np){
                found = first;
                break;
            }
        }
    }

    if(found > 0){
        for(i = found; i < found + np; i++)
            freeRamFrames[i]=(unsigned char)0;

        allocSize[found]=np;
        addr = found*PAGE_SIZE;
    }
    else
        addr = 0;

    spinlock_release(&freemem_lock);

    return addr;    
}
/**
 * @brief Trova la prime pagine libere
 * 
 * @param npages 
 * @return paddr_t 
 */
paddr_t coremap_getppages(unsigned long npages){
    return 0;
}

int freepages(paddr_t addr, unsigned long npages){
    return 0;
}



//Deve trovare un segmento libero
//void coremap_find_free(void)
//{
    //cercare un frame libero
    //se libero chiama la funzione che lo occupi
    //se non c'è nessun frame libero
    //chiama la funzione che faccia swap out di qualcosa
//}

//Deve trovare N segmenti liberi contigui
//void coremap_find_nfree(void /*int num_seg*/)
//{
    //cerca n frame contigui liberi
    //se li trova, chiama la funzione che li occupi
    //se non li trova
    //chiama la funzione che li trovi
//}
