// vmstats.c: code for tracking stats

#include "vmstats.h"

/**
 * @brief Inizializzazione della struttura dati. Occorre modificare la struttura dati 
 * 
 * @return struct tlb_report 
 */
struct tlb_report vmstats_init(void){
    struct tlb_report report;

    report.tlb_fault = 0;
    report.tlb_faultFree = 0;
    report.tlb_faultReplacement = 0;

    return report;
}