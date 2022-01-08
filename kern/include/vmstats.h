#ifndef _VM_STATS_H_
#define _VM_STATS_H_

#include <types.h>
#include <kern/errno.h>


#define DEBUG_PAGING 1

typedef struct tlb_report{
    unsigned int tlb_fault; // Numero totale di tlb fault
    unsigned int tlb_faultFree; // Numero toale di tlb fault con celle libere della tlb
    unsigned int tlb_faultReplacement; // Numero di tlb fault con rimpiazzamento
    unsigned int tlb_invalidation;  // Numero di tlb invalidation
    unsigned int tlb_reload; // Numero di tlb reload

    unsigned int pf_zero; // Numero di tlb misses che richiedono una pagina riempita di zeri
    unsigned int pf_disk; // Numero di tlb misses che richiedono una pagina che e' stata caricata da disco
    unsigned int pf_elf; // Numero di tlb misses che richedono una pagi a dall' elf file
    unsigned int pf_swapin;
    unsigned int pf_swapout;

}tlb_report;

extern struct tlb_report vmstats_report;

// Inizializza i contatori della struttura dati
struct tlb_report vmstats_init(void);

#endif