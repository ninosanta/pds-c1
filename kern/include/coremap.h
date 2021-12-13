#include <types.h>
#include <vm.h>


int coremap_init(size_t ram_size);


int vm_is_active(void);

/* Fault handling function called by trap code */
//int vm_fault(int faulttype, vaddr_t faultaddress);


/* Allocate kernel heap pages (called by kmalloc/kfree) */
vaddr_t alloc_kpages(unsigned npages);

/* Free kernel heap pages (called by kmalloc/kfree) */
void free_kpages(vaddr_t addr);

/* TLB shootdown handling called from interprocessor_interrupt */
 //void vm_tlbshootdown(const struct tlbshootdown *); 

