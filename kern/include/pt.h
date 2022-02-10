#include <types.h>
#include <lib.h>

typedef struct pt_fifo_node pt_FIFO;

struct ipt_t{
        pid_t pid;
        vaddr_t vaddr;
        bool invalid;
        bool readonly;
        unsigned char flags;
        unsigned int counter;
};


int pt_init (void);

void pt_add_entry( vaddr_t vaddr , paddr_t paddr, pid_t pid, /* bool readonly,  */unsigned char flag );

int pt_replace_entry( pid_t pid); 

unsigned int pt_get_paddr ( vaddr_t vaddr, pid_t pid , paddr_t paddr);

int pt_remove_entry (vaddr_t vaddr, pid_t pid);

void pt_destroy ( void );

vaddr_t pt_getVaddrByIndex(int index); 

unsigned char pt_getFlagsByIndex(int index); 

pid_t pt_getPidByIndex(int index);

//struct ipt_t pt_get_entry (void); //funzione usata per verificare la presenza d>



