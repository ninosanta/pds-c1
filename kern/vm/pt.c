// pt.c: page tables and page table entry manipulation go here 

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


/* 
void pt_get_entry 

 int pt_init( unsigned int lenght) {
     //alloca la page table con dimensione della memoria fisica
 }

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