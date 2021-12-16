#include <types.h>


struct ipt_t{
        pid_t pid;
        vaddr_t vaddr;
        bool invalid;
        bool readonly;
};


int pt_init (void);

void pt_add_entry( vaddr_t vaddr , paddr_t paddr, pid_t pid, bool readonly );

int pt_get_paddr ( vaddr_t vaddr, pid_t pid , paddr_t* paddr);

int pt_remove_entry (vaddr_t vaddr, pid_t pid);

void pt_destroy ( void );

struct ipt_t pt_get_entry (void); //funzione usata per verificare la presenza d>



