// pt.c: page tables and page table entry manipulation go here

#include <types.h>
#include <kern/errno.h>
#include <lib.h>
#include <vm.h>
#include <pt.h>
#include <spinlock.h>



// Variabili globali
static struct spinlock freemem_lock = SPINLOCK_INITIALIZER; // Gestione in mutua esclusione
//static struct spinlock stealmem_lock = SPINLOCK_INITIALIZER;

static unsigned int nRamFrames = 0; // Vettore dinamico della memoria Ram assegnata al Boot (dipende da sys161.conf)


//static struct spinlock pt_lock = SPINLOCK_INITIALIZER;

//INVERTED PAGE TABLE  --> PID, P, invalid bit, readonly bit
//--> one entry for each real page in memory
//quando ho un TLB MISS devo vedere dentro alla inverted page table, e inserire una nuova entry nella TLB
//quando faccio partire un processo, devo caricare un processo in memoria fisica
//quando lo metto in memoria fisica, devo inserire le entry nella inverted page table
//per la ricerca nella inverted page table bisogna anche creare un meccanismo di hash per l'inserimento e la ricerca delle entry più veloce

static struct ipt_t *ipt = NULL;


//page table
//page table entry --> 20 bit di indirizzo fisico e 12 bit di attributi


//però noi dobbiamo fare quella INVERTED quindi sono da adattare le seguenti cose
//usata dalle funzioni in address space


//Inizializzaizone della Inverted Page Table
int pt_init ( void ){
    unsigned int i ;

    //Numero di frame della RAM
    nRamFrames = (unsigned int)ram_getsize() / PAGE_SIZE ;

    //alloca la page table con dimensione della memoria fisica
     if( ! (ipt = kmalloc(sizeof(struct ipt_t) * nRamFrames))){
        ipt =NULL;
        return 1; //Out of Memory
    }

    spinlock_acquire( &freemem_lock ); 
    for ( i = 0 ; i < nRamFrames ; i++ ){
        ipt[i].invalid = 1;
        ipt[i].readonly= 0;
    }
    spinlock_release( &freemem_lock);
    return 0;
}

void pt_add_entry ( vaddr_t vaddr , paddr_t paddr, pid_t pid, bool readonly ){

    unsigned int index = ((unsigned int)paddr)/PAGE_SIZE;

    KASSERT( index <  nRamFrames );
    spinlock_acquire( &freemem_lock); 
    ipt[index].vaddr = vaddr;
    ipt[index].pid = pid;
    ipt[index].readonly = readonly;
    ipt[index].invalid = 0;
    spinlock_release(&freemem_lock); 

}

 //int pt_replace_entry( paddr_t paddr, vaddr_t vaddrOut, vaddr_t vaddrIn, pid_t pidOut, pid_t pidIn , bool readonly){

     /*for( i = 0 ; i < nRamFrames ; i++){
         if( ipt[i].invalid == 1){}
     }
*/
    //quando viene sostiutuita una pagina nella memoria fisica
    //viene rimpiazzata la entry nella page table
   // return 0; 
 //}

paddr_t pt_get_paddr ( vaddr_t vaddr, pid_t pid ){
    unsigned int i = 0 ;
    paddr_t p; 


    spinlock_acquire( &freemem_lock ); 
    while ( i < nRamFrames){
        if( ipt[i].pid == pid && ipt[i].vaddr == vaddr && ipt[i].invalid == 0){
            p = (i*PAGE_SIZE) ;
            return p ;
         }
        i++;
    }
    spinlock_release( &freemem_lock ); 

    return 1;  // non presente in memoria
}

int pt_remove_entry (vaddr_t vaddr, pid_t pid){
    unsigned int i = 0 ;
    
    spinlock_acquire( &freemem_lock); 
    while ( i < nRamFrames){
        if( ipt[i].pid == pid && ipt[i].vaddr == vaddr && ipt[i].invalid == 0){
            ipt[i].invalid = 1 ;
            return 0;
        }
        i++;
    }
    spinlock_release( &freemem_lock); 

    return 1; 
}

void pt_destroy ( void ){
    unsigned int i = 0 ;

    spinlock_acquire( &freemem_lock); 
    for (i=0; i<nRamFrames; i++){
        kfree((void*)&ipt[i]);
    }
    spinlock_release( &freemem_lock); 
    
}
//struct ipt_t* pt_get_entry (pid_t pid, vaddr_t vaddr){ //riceve pid del processo e indirizzo virtuale
//spinlock per page table?
    //int count = 0 ;
    /*while ( count <  ){
       if ( ipt-> ipt_e[count].pid == pid ){
           if ( ipt-> ipt_e[count].vaddr == vaddr){
               if( ipt->ipt_e[count].invalid == 0)
                    return ipt->ipt_e;
                else return PT_INVALID_ENTRY;
           }
       }
       count ++;
    }
    return PT_ENTRY_NOT_FOUND; */
//se c'è la entry corruspondente ritorna il puntatore corrispondente
//se non c'è ritorna un qualcosa che indichi che non c'è

//questa funzione può essere chiamata per verificare se una pagina è presente in memoria fisica
//}
/*


int pt_
 add_entry ()
 {
     //quando viene caricata una pagina fisica nuova
     //viene aggiunta unan nuova entry nella tlb
 }

 int pt_replace_entry( {

    //quando viene sostiutuita una pagina nella memoria fisica
    //viene rimpiazzata la entry nella page table
 }


 int pt_get_paddr(){
     //viene ricevuto un pid + page num + d
     //si trova la entry corrispondente nella page table
     //si trova la pagina in memoria fisica
     //si vuole ritornare l'indirizzo fisico
 }

 int pt_remove_entry()
{}

int pt_destroy (){}

//get virtual address by index

//get physical address by index
//get pid by index

//get flags(?) by index
//set TLB index
//set flags at index

 */

