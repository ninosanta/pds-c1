// vmstats.c: code for tracking stats

#include "vmstats.h"

#include <spinlock.h>
/**
 * @brief Inizializzazione della struttura dati. Occorre modificare la struttura dati
 *
 * @return struct tlb_report
 */
struct tlb_report vmstats_init(void)
{
    struct tlb_report report;

    report.tlb_fault = 0;
    report.tlb_faultFree = 0;
    report.tlb_faultReplacement = 0;
    report.tlb_invalidation = 0;
    report.tlb_reload = 0;

    report.pf_zero = 0;
    report.pf_disk = 0;
    report.pf_elf = 0;
    report.pf_swapin = 0;
    report.pf_swapout = 0;

    return report;
}