/**
 * @file vmstats.c
 * @author your name (you@domain.com)
 * @brief code for tracking stats
 * @version 0.1
 * @date 2022-02-23
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "vmstats.h"
#include <spinlock.h>
#include <lib.h>

static struct spinlock stats_lock = SPINLOCK_INITIALIZER; 

/**
 * @brief Inizializzazione della struttura dati. Occorre modificare la struttura dati
 *
 * @return struct tlb_report
 */
struct tlb_report vmstats_init(void)
{
    struct tlb_report report;
    spinlock_acquire(&stats_lock); 
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
    spinlock_release(&stats_lock); 

    return report;
}

/**
 * @brief Funzioni per l'incremento dei singoli valori 
 * 
 */

void vmstats_report_tlb_fault_increment(void){
    spinlock_acquire( &stats_lock); 
    vmstats_report.tlb_fault++; 
    spinlock_release(&stats_lock); 
}
void vmstats_report_tlb_faultFree_increment(void){
    spinlock_acquire( &stats_lock); 
    vmstats_report.tlb_faultFree++;
    spinlock_release(&stats_lock);  
}
void vmstats_report_tlb_faultReplacement_increment(void){
    spinlock_acquire( &stats_lock); 
    vmstats_report.tlb_faultReplacement++; 
    spinlock_release(&stats_lock); 
}
void vmstats_report_tlb_invalidation_increment(void){
    spinlock_acquire( &stats_lock); 
    vmstats_report.tlb_invalidation++; 
    spinlock_release(&stats_lock); 
}
void vmstats_report_tlb_reload_increment(void){
    spinlock_acquire( &stats_lock); 
    vmstats_report.tlb_reload++; 
    spinlock_release(&stats_lock); 
}
void vmstats_report_pf_zero_increment(void){
    spinlock_acquire( &stats_lock); 
    vmstats_report.pf_zero++; 
    spinlock_release(&stats_lock); 
}
void vmstats_report_pf_disk_increment(void){
    spinlock_acquire( &stats_lock); 
    vmstats_report.pf_disk++; 
    spinlock_release(&stats_lock); 
}
void vmstats_report_pf_elf_increment(void){
    spinlock_acquire( &stats_lock); 
    vmstats_report.pf_elf++; 
    spinlock_release(&stats_lock); 
}
void vmstats_report_pf_swapin_increment(void){
    spinlock_acquire( &stats_lock); 
    vmstats_report.pf_swapin++; 
    spinlock_release(&stats_lock); 
}
void vmstats_report_pf_swapout_increment(void){
    spinlock_acquire( &stats_lock); 
    vmstats_report.pf_swapout++; 
    spinlock_release(&stats_lock); 
}

/**
 * @brief Stampa le statistiche
 * 
 */
void vmstats_report_print(void){
    kprintf("\nStatistics of projects\n\n");
	kprintf("tlb fault: %d\n"
		"tlb fault with free space: %d\n"
		"tlb fault with replace: %d\n"
		"tlb fault with invalidation: %d\n"
		"tlb fault with reload: %d\n"
		"page fault with zero: %d\n"
		"page table with disk: %d\n"
		"page table with elf: %d\n"
		"page table with swapin: %d\n"
		"page table with swapout: %d\n"
		
		"\n\n", vmstats_report.tlb_fault, 
		vmstats_report.tlb_faultFree, 
		vmstats_report.tlb_faultReplacement,
		vmstats_report.tlb_invalidation,
		vmstats_report.tlb_reload,
		vmstats_report.pf_zero,
		vmstats_report.pf_disk,
		vmstats_report.pf_elf,
		vmstats_report.pf_swapin,
		vmstats_report.pf_swapout
		);
} 