// vm_tlb.c: code for manipulating the tlb (including replacement)



//cose che leggendo meglio è più prbabile che io debba mettere

/* int tlb_get_rr_victim(void) { 
  int victim; 
  static unsigned int next_victim = 0; 
  victim = next_victim; 
  next_victim = (next_victim + 1) % NUM_TLB; 
  return victim; 
}  
int tlb_init(void) { 
    
}
int tlb_write_entry(void) { }
int tlb_replace_entry (void) { }

 */


/* cose probabilment einutili che ho scritto io
//modificare le entry della tlb in modo che segni quali pagine sono RW e RO
//se si tenta di accedere ad RO allora si generi un'eccezione senza far crashare il kernel
  --> dirty bit serve per segnare una entry come read only (0) o read/write (1)

//TLB init
//TLB destroy
//TLB_add_entry 
//TLB_replace entry
//TLB_miss
//TLB_find
//TLB _destroy_entry 

//TLB check process --> bisogn aassicurarsi che tutte le entri nella TLB siano appartenenti al processo 
*/

