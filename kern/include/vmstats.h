#ifndef _VM_STATS_H_
#define _VM_STATS_H_

#include <types.h>
#include <kern/errno.h>

#define DEBUG_PAGING 1

typedef struct tlb_report
{
    unsigned int tlb_fault;            // number of TLB misses that have occurred
    unsigned int tlb_faultFree;        /* number of TLB misses for which there was free space in the TLB
                                        * to add the new TLB entry (i.e., no replacement was required) */
    unsigned int tlb_faultReplacement; // number of TLB misses with replacement
    unsigned int tlb_invalidation;     // number of times the TLB was invalidated
    unsigned int tlb_reload;           // number of TLB misses for pages that were already in memory

    unsigned int pf_zero;    // number of TLB misses that required a new page to be zero-filled
    unsigned int pf_disk;    // number of TLB misses that required a page to be loaded from disk
    unsigned int pf_elf;     // number of page faults that require getting a page from the ELF file
    unsigned int pf_swapin;  // number of page faults that require getting a page from the swap file
    unsigned int pf_swapout; // number of page faults that require writing a page to the swap file

} tlb_report;

extern struct tlb_report vmstats_report;

// Inizializza i contatori della struttura dati
struct tlb_report vmstats_init(void);
void vmstats_report_tlb_fault_increment(void);
void vmstats_report_tlb_faultFree_increment(void);
void vmstats_report_tlb_faultReplacement_increment(void);
void vmstats_report_tlb_invalidation_increment(void);
void vmstats_report_tlb_reload_increment(void);
void vmstats_report_pf_zero_increment(void);
void vmstats_report_pf_disk_increment(void);
void vmstats_report_pf_elf_increment(void);
void vmstats_report_pf_swapin_increment(void);
void vmstats_report_pf_swapout_increment(void);
void vmstats_report_print(void); 

#endif