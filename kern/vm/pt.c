/**
 * @file pt.c
 * @author your name (you@domain.com)
 * @brief code for managing the page table (INVERTED PAGE TABLE)
 * @version 0.1
 * @date 2022-02-23
 * 
 * @copyright Copyright (c) 2022
 * 
 */

//Librerie
#include <types.h>
#include <kern/errno.h>
#include <lib.h>
#include <vm.h>
#include <spinlock.h>
#include <clock.h>

#include "coremap.h"
#include "pt.h"
#include "vmstats.h"

// Variabili globali
static struct spinlock stealmem_lock = SPINLOCK_INITIALIZER; // Gestione in mutua esclusione
static unsigned int nRamFrames = 0; // Vettore dinamico della memoria Ram assegnata al Boot (dipende da sys161.conf)
static struct ipt_t *ipt = NULL;


/************************************************************
 *                                                          *
 * Implementazione delle funzioni                           *
 *                                                          *
 ************************************************************/

/**
 * @brief Inizializzazione della Invreted Page Table
 * 
 * @return int 
 */
int pt_init(void)
{
    unsigned int i;

    // Numero di frame della RAM
    nRamFrames = (unsigned int)ram_getsize() / PAGE_SIZE;

    // Alloca la page table con dimensione della memoria fisica
    if (!(ipt = kmalloc(sizeof(struct ipt_t) * nRamFrames)))
    {
        ipt = NULL;
        return 1; // Out of Memory
    }

    spinlock_acquire(&stealmem_lock);

    // Inizializzazione delle delle della ipt
    for (i = 0; i < nRamFrames; i++)
    {
        ipt[i].pid = -1;
        ipt[i].invalid = 1;
        ipt[i].flags = 0;
        ipt[i].counter = 0;
    }

    spinlock_release(&stealmem_lock);
    return 0;
}

/**
 * @brief Funzione che aggiorna il valore che tiene conto delle pagine
 *         più vecchie per sostituzione FIFO
 */
static void upgrade_counter(void)
{
    unsigned int i;

    if (ipt == NULL)
        return;

    // Aggiornamento del counter d'uso della pagina
    for (i = 0; i < nRamFrames; i++)
        if (ipt[i].invalid == 0 && ipt[i].pid != -1)
            ipt[i].counter++;
}

/**
 * @brief Funzione che aggiunge aggiunge una entry nella pagetable all'indirizzo paddr
 * 
 * @param vaddr 
 * @param paddr 
 * @param pid 
 * @param flag 
 * @return int 
 */
int pt_add_entry(vaddr_t vaddr, paddr_t paddr, pid_t pid, unsigned char flag)
{
    unsigned int index = ((unsigned int)paddr) / PAGE_SIZE;

    KASSERT(index < nRamFrames);

    spinlock_acquire(&stealmem_lock);

    ipt[index].vaddr = vaddr;
    ipt[index].pid = pid;
    ipt[index].invalid = 0;
    ipt[index].flags = flag;
    ipt[index].counter = 0;

    upgrade_counter();

    spinlock_release(&stealmem_lock);

    return 0;
}

/**
 * @brief Funzione che, dato il pid del processo, cerca una pagina del processo stesso da sostituire all'interno della page table
 *        Restituisce l'indice della cella da sostituire
 * @param pid 
 * @return int 
 */
int pt_replace_entry(pid_t pid)
{
    unsigned int i;
    unsigned int max = 0;
    int replace_index = nRamFrames;

    spinlock_acquire(&stealmem_lock);

    for (i = 0; i < nRamFrames; i++)
    {
        //Cerca il massimo valore di counter ( strategia FIFO )
        // e ne salva l'indice
        if (max < (ipt[i].counter) && pid == ipt[i].pid)
        {
            max = ipt[i].counter; 
            replace_index = i;
        }
    }
 
    spinlock_release(&stealmem_lock);

    return replace_index;
}

/**
 * @brief Funzione che cerca la pagina in memoria più vecchia in assoluto, e ne restituisce l'indice
 * 
 * @return int 
 */
int pt_replace_any_entry(void){
    unsigned int i;
    unsigned int max = 0;
    int replace_index = nRamFrames;

    spinlock_acquire(&stealmem_lock);

    for (i = 0; i < nRamFrames; i++)
    {
        //Cerca il massimo valore di counter ( strategia FIFO )
        // e ne salva l'indice
        if (max < (ipt[i].counter))
        {
            max = ipt[i].counter; 
            replace_index = i;
        }
    }
    spinlock_release(&stealmem_lock);

    return replace_index;
}

/**
 * @brief Funzione che, dato il pid del processo e il virtual address della pagina, 
 *        restituisce l'indirizzo fisico tramite riferimento 
 *        Restituisce 0 se la pagina non si trova in memoria, 1 in caso di successo
 * 
 * @param vaddr 
 * @param pid 
 * @param paddr 
 * @return unsigned int 
 */
unsigned int pt_get_paddr ( vaddr_t vaddr, pid_t pid , paddr_t* paddr){
    unsigned int i = 0;

    spinlock_acquire( &stealmem_lock );
    /* Scansione linerare ricercando la corrispondenza tra i seguenti paramentri:
        - Pid
        - Indirizzo virtuale (vaddr)
        - validazione della pagina */
    while ( i < nRamFrames){
        if( ipt[i].pid == pid && ipt[i].vaddr == vaddr && ipt[i].invalid == 0){
            // L'indirizzo fisico corrisponde all'indice della pagina per la 
            // dimensione della pagina stessa
            *paddr= (i*PAGE_SIZE);
            spinlock_release(&stealmem_lock); 
            return 1;
         }
        i++;
    }
    spinlock_release( &stealmem_lock ); 
    
    return 0;  // La pagina richiesta non è presente nella page table
} 

/**
 * @brief Invalida la entry corrispondente all'indirizzo passato come parametro
 * 
 * @param replace_index 
 */
void pt_remove_entry(int replace_index)
{
    spinlock_acquire(&stealmem_lock);
    ipt[replace_index].pid = -1;
    ipt[replace_index].flags = 0;
    ipt[replace_index].vaddr = 0;
    ipt[replace_index].invalid = 1;
    spinlock_release(&stealmem_lock);
}

/**
 * @brief Invalida tutte le entry relative al pid passato come parametro
 * 
 * @param pid 
 */
void pt_remove_entries(pid_t pid)
{
    unsigned int i;

    KASSERT(pid >= 0);

    spinlock_acquire(&stealmem_lock);

    // Ricerca tutte le corrispondeze delle pagine associate al pid 
    // ricevuto come parametro per invalidarle
    for (i = 0; i < nRamFrames; i++)
    {
        if (ipt[i].pid == pid)
        {
            ipt[i].pid = -1;
            ipt[i].flags = 0;
            ipt[i].vaddr = 0;
            ipt[i].invalid = 1;
            
            coremap_freepages(i * PAGE_SIZE);
        }
    }

    spinlock_release(&stealmem_lock);
}

/**
 * @brief Distrugge la page table
 * 
 */
void pt_destroy(void)
{
    unsigned int i = 0;

    spinlock_acquire(&stealmem_lock);
    for (i = 0; i < nRamFrames; i++)
    {   
        // De-allocazione della memoria
        kfree((void *)&ipt[i]); 
    }
    spinlock_release(&stealmem_lock);
}

/**
 * @brief Ottiene i flag, dato l'indice del frame
 * 
 * @param index 
 * @return unsigned char 
 */
unsigned char pt_getFlagsByIndex(int index)
{
    return ipt[index].flags;
}

/**
 * @brief Ottiene il Virtual address del frame passato
 * 
 * @param index 
 * @return vaddr_t 
 */
vaddr_t pt_getVaddrByIndex(int index)
{
    return ipt[index].vaddr;
}

/**
 * @brief Ottiene il pid della pagina salvata all'indice passato come parametro
 * 
 * @param index 
 * @return pid_t 
 */
pid_t pt_getPidByIndex(int index)
{
    return ipt[index].pid;
}

/**
 * @brief Setta i flag di una entry della page table
 * 
 * @param index 
 * @param val 
 */
void pt_setFlagsAtIndex(int index, unsigned char val)
{
    ipt[index].flags |= val;
}

// Alternativa di una inverted page table con tempi di ricerca minori
#ifdef FIFOSCHEDULING

struct ipt_fifo_node
{
    unsigned char index_ipt;
    struct ipt_fifo *next;
};

struct ipt_fifo
{
    struct ipt_fifo_node *head;
    struct ipt_fifo_node *tail;

    unsigned int size;
};

static struct ipt_fifo_node *fifo_node_Create(unsigned int index_ipt)
{
    struct ipt_fifo_node *node;

    node = (struct pt_fifo_node *)kmalloc(sizeof(struct ipt_fifo_node));
    node->index_ipt = index_ipt;
    node->next = NULL;

    return node;
}

static void fifo_node_Insert(unsigned int index_ipt)
{
    struct ipt_fifo_node *node = fifo_node_Create(index_ipt);
    if (list_replacement != NULL)
    {
        if (list_replacement->head == NULL)
        {
            list_replacement->head = node;
            list_replacement->tail = node;
        }
        else
        {
            list_replacement->tail->next = node;
        }
    }
}

static int fifo_node_Remove()
{
    struct ipt_fifo_node *node = NULL;
    int value = -1;

    if (list_replacement != NULL)
    {
        if (list_replacement->head != NULL)
        {
            node = list_replacement->head;
            value = node->index_ipt;
            list_replacement->head = list_replacement->head->next;

            kfree(node);
        }
    }

    return value;
}
#endif
