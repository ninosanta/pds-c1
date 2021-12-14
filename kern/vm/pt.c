// pt.c: page tables and page table entry manipulation go here 

#include <types.h>
#include <vm.h>
#include <pt.h>

//INVERTED PAGE TABLE  --> PID, P, invalid bit, readonly bit
//--> one entry for each real page in memory 
//quando ho un TLB MISS devo vedere dentro alla inverted page table, e inserire una nuova entry nella TLB
//quando faccio partire un processo, devo caricare un processo in memoria fisica
//quando lo metto in memoria fisica, devo inserire le entry nella inverted page table
//per la ricerca nella inverted page table bisogna anche creare un meccanismo di hash per l'inserimento e la ricerca delle entry più veloce



//page table
//page table entry --> 20 bit di indirizzo fisico e 12 bit di attributi 

//però noi dobbiamo fare quella INVERTED quindi sono da adattare le seguenti cose
//usata dalle funzioni in address space


//Inizializzaizone della Inverted Page Table
int pt_init ( int num_ram_pages){
    //alloca la page table con dimensione della memoria fisica
    return num_ram_pages; //stringa messa per evitare warning
}


void pt_get_entry (void){ //riceve address space e indirizzo virtuale
// prende la page table
//se c'è la entry corruspondente ritorna il puntatore corrispondente
//se non c'è ritorna un qualcosa che indichi che non c'è 


//questa funzione può essere chiamata per verificare se una pagina è presente in memoria fisica

}
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