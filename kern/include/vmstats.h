#ifndef _VM_STATS_H_
#define _VM_STATS_H_

#include <types.h>
#include <kern/errno.h>


#define DEBUG_PAGING 1

typedef struct tlb_report{
    unsigned int tlb_fault; // Numero totale di tlb fault
    unsigned int tlb_faultFree; // Numero toale di tlb fault con celle libere della tlb
    unsigned int tlb_faultReplacement; // Numero di tlb fault con rimpiazzamento
    
}tlb_report;

extern struct tlb_report vmstats_report;

// Inizializza i contatori della struttura dati
struct tlb_report vmstats_init(void);

#endif