// vmstats.c: code for tracking stats

#include "vmstats.h"

struct tlb_report vmstats_init(void){
    struct tlb_report report;

    report.tlb_fault = 0;
    report.tlb_faultFree = 0;
    report.tlb_faultReplacement = 0;

    return report;
}