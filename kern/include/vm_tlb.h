
//ENTRY DELLA TLB
struct tlb_t {
        vaddr_t virtual_page_num; 
        paddr_t physical_page_num; 
        //bool global;//UNUSED //1 ignora i bit del pid
        bool valid;  //1 indica come valida la entry
        bool dirty;  //0 (read only) 1( read write)
        //bool nocache; //UNUSED
        //pid_t pid; //UNUSED //address space id che, se indica che la entry 
        //appartiene al processo corrente, permette ad essa 
        //di rimanere nella tlb
};