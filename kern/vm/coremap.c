/**
 * @file coremap.c
 * @author your name (you@domain.com)
 * @brief Managing kernel memory with a coremap: Tenere traccia dei frame fisici.
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

/**
 * @brief La primo passo del sistema di memoria virtuale OS161 è la gestione delle pagine fisiche.
 * In generale, possiamo impacchettare le informazioni di una pagina fisica in una struttura (struct coremap_entry) e usalra per
 * rappresentare una pagina fisica. Usiamo un array di array struct coremap_entry per mantenere tutte le informazioni sulle pagine
 * fisiche
 *
 */

// Define
#define RAMFRAME_FREE 0
#define RAMFRAME_ALLOCATED 1
#define ALLOCSIZE_DEFAULT 0 // Numero di pagine allocate di default

// AllocTable
#define ALLOCTABLE_ENABLE 1
#define ALLOCTABLE_DISABLE 0

// Variabili globali
static struct spinlock pt_lock = SPINLOCK_INITIALIZER; 
static struct spinlock stealmem_lock = SPINLOCK_INITIALIZER;

static unsigned int nRamFrames = 0;         // Dimensione della memoria Ram assegnata al Boot (dipende da sys161.conf)
static unsigned char *freeRamFrames = NULL; // Vettore di marcaggi delle locazioni occupate (valore 1) e libere (valore 0)
static unsigned long *allocSize = NULL;     // Dimensione allocata alle varie pagine
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
 * Response: non è possibile trascurare le prime celle del vettore in quanto i vettori sono allocati dinamicamente in base al numero di frame totali.
 *          Tuttavia è possibile simulare un'allocazione della memoria totale, calcolare l'effettivo numero di frame e deallocare per riallocare
 *          (avremo un po meno di quello prefissato in quando il numero di frame sarà minore).
 *
 */
int coremap_init(void)
{
    unsigned int i;

    nRamFrames = ((int)ram_getsize() / PAGE_SIZE);

    // Allocazione dei vettori freeRamFrame e allocSize
    if (!(freeRamFrames = kmalloc(sizeof(unsigned char) * nRamFrames)) ||
        !(allocSize = kmalloc(sizeof(unsigned long) * nRamFrames)))
    {
        freeRamFrames = NULL;
        allocSize = NULL;

        return ENOMEM;
    }

    // Inizializzazione dei vettori freeRamFrame e allocSize
    for (i = 0; i < nRamFrames; i++)
    {
        freeRamFrames[i] = (unsigned char)RAMFRAME_FREE;
        allocSize[i] = (unsigned long)ALLOCSIZE_DEFAULT;
    }

    spinlock_acquire(&pt_lock);
    allocTableActive = ALLOCTABLE_ENABLE; // Tabella allocata correttamente
    spinlock_release(&pt_lock);

    return COREMAP_INIT_SUCCESS;
}

/**
 * @brief Analizza in mutua esclusione la variabile denominata active
 *
 * @return int
 */
int coremap_isTableActive(void)
{
    int active;

    spinlock_acquire(&pt_lock);
    active = allocTableActive;
    spinlock_release(&pt_lock);

    return active;
}

/**
 * @brief Ricerca un numero di pagine consecutive libere
 *
 * @param npages
 * @return paddr_t
 */
static paddr_t getfreeppages(unsigned long npages)
{
    paddr_t addr;
    int i;
    long
        first,
        found,
        np = (long)npages;

    if (!coremap_isTableActive())
        return ALLOCTABLE_DISABLE;

    spinlock_acquire(&pt_lock);

    // Ricerca lineare degli intervalli liberi
    for (i = 0, first = found = -1; i < (int)nRamFrames; i++)
    {
        if (freeRamFrames[i])
        {
            if (!i || !freeRamFrames[i - 1])
                first = i; // Imposta il primo intervallo libero
            if (i - first + 1 >= np)
            {
                found = first;
                break;
            }
        }
    }

    if (found > 0)
    {
        for (i = found; i < found + np; i++)
            freeRamFrames[i] = (unsigned char)RAMFRAME_FREE;

        allocSize[found] = np;
        addr = (paddr_t)found * PAGE_SIZE;
    }
    else
        addr = 0;

    spinlock_release(&pt_lock);

    return addr;
}

/**
 * @brief This function should simply get the next available physical page and return it. If there are no pages available you could return 0 so that alloc_kpages
 *      will attempt to swap or otherwise free a page, or you could do that within this function. This is the main interface to the Coremap. This is where
 *      you take in a VPN find an available PFN and pass it back to alloc_kpages. Remember that your Coremap is a hashtable but with some special properties.
 *      You CANNOT have more VPN in the hashtable than PFN unless some address spaces are sharing physical memory. This differs from a normal hash table in that
 *      chaining as a method of collision resolution becomes complicated. You must point back into your array of nodes rather than pointing to a linked list.
 *      What you store in your Coremap is really up to you. You could store ASIDs and PIDs, you could throw pointers to thread structures in there etc.
 *      Although I would suggest PIDs over thread pointers since you already build a level of abstraction for finding threads by PID for waitpid.
 *
 * @param npages
 * @return paddr_t
 */
paddr_t coremap_getppages(unsigned long npages)
{
    paddr_t addr;

    // Trova il primo slot sufficientemente grande di pagine contigue
    if (!(addr = getfreeppages(npages)))
    {
        spinlock_acquire(&stealmem_lock);
        addr = ram_stealmem(npages);
        spinlock_release(&stealmem_lock);
    }
    if (addr && coremap_isTableActive())
    {
        spinlock_acquire(&pt_lock);
        allocSize[addr / PAGE_SIZE] = npages; // Numero di pagine allocate
        spinlock_release(&pt_lock);
    }

    return addr;
}

/**
 * @brief Dato l'indirizzo fisico, libera le pagine assegnate a paratire da quell'indirizzo, se allocate insieme
 *
 * @param addr
 * @return int 
 */
int coremap_freepages(paddr_t addr)
{
    long i,
        first = addr / PAGE_SIZE,
        np;

    KASSERT(allocSize != NULL);
    KASSERT(nRamFrames > ((unsigned int)first));

    np = allocSize[first];

    spinlock_acquire(&pt_lock);
    for (i = first; i < first + np; i++)
        freeRamFrames[i] = (unsigned char)RAMFRAME_ALLOCATED;
    spinlock_release(&pt_lock);

    return 1;
}

