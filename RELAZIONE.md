Argomento | Team 
--------|---------
Project C1 - Virtual Memory with Demand Paging on OS/161 | [@martact](https://github.com/martact) [@llriccardo98ll](https://github.com/llriccardo98ll) [@ninosanta](https://github.com/ninosanta) 


# Virtual Memory with Demand Paging on OS/161

## Abstract

L'obiettivo del progetto è quello di accrescere le funzionalità del modulo base di gestione della memoria (DUMBVM) in OS/161, sostituendolo con un più potente Virtual Memory Manager che sfrutti delle Process Page Tables. In tal senso, i frame fisici saranno assegnati su richiesta e le pagine virtuali verranno caricate anch'esse su richiesta. "Su richiesta" significa che la pagina sarà allocata la prima volta che l'applicazione tenta di utilizzarla. Quindi, le pagine che non vengono mai utilizzate da un'applicazione non dovranno mai essere caricate in memoria e non dovranno consumare un frame fisico.

Il sistema implementerà, con una Page Table, la richiesta/rimpiazzo delle pagine in modo tale che:
* Il kernel non crashi se la TLB si riempe.
* Programmi che hanno un address space più grande della memoria fisica possano essere eseguiti, a patto che non tocchino più pagine di quante ne rientrano nella memoria fisica.
* Grazie al Page Replacement un nuovo frame possa essere trovato qualora non ci fossero più frame disponibili.

Quando si verifica una TLB miss, il gestore delle eccezioni di OS161 deve caricare una entry appropriata nella TLB. Se c'è spazio libero nel TLB, la nuova entry finirà nello spazio libero. Altrimenti, applicando una politica di rimpiazzo a round-robin simil FIFO, OS161 sceglierà una entry TLB da rimuovere per creare spazio alla nuova entry, garantendo al contempo che tutte le entry della TLB si riferiscano al processo attualmente in esecuzione. 

## Flusso d'esecuzione di un programma utente

### runprogram.c

Questo file è composto da un'unica funzione, la `runprogram()`, la quale riceve come unico parametro il nome del programma. Tale programma verrà aperto in sola lettura dalla `vfs_open()` e un nuovo address space verrà creato e attivato per esso, rispettivamente tramite le funzioni `as_create()` e `as_activate()` (definite in [addrspace.c](#addrspacec)). Dopodiché, tramite la funzione `load_elf()` (definita in [loadelf.c](#loadelfc)), vengono definiti i segmenti di dato e utente del programma e ne viene caricato l'eseguibile nell'address space corrente settandone anche l'entrypoint del programma. In fine, lo stack segment verrà definito tramite la funzione `as_define_stack()` e finalmente il processo potrà partire tramite la funzione `enter_new_process()`.

Rispetto al vecchio `runprogram.c` la differenza sostanziale è che qui non chiudiamo il file ELF perché le pagine verranno caricate su richiesta. Quindi, il file ELF verrà chiuso solamente quando il programma avrà terminato l'esecuzione.

### loadelf.c

Le funzione `load_elf()` in precedenza si occupava del caricamento dell'intero file in memoria ma, seguendo la politica della paginazione su richiesta, non occorre più sia così.

Inoltre, non è previsto l'utilizzo delle funzione `load_segment()` e tutto il necessario sarà fatto nella `load_elf()` che, come già detto, ritornerà l'entrypoint del processo che verrà utilizzato per avviare il processo. Qui, inoltre, verrà scandito l'header dell'eseguibile e verranno definite le regioni dell'addess space tramite la funzione `as_define_region()` preparandole tramite la funzione `as_prepare_load()` (definite in [addrspace.c](#addrspacec)). 

## Flusso del caricamento di una pagina dopo un TLB fault

In caso di TLB miss, viene generato un Page Fault i.e., un'eccezione di indirizzo che indica la mancanza della pagina richiesta nella TLB. E questa eccezione viene gestita attraverso la funzione `vm_fault()` (definita in [addrspace.c](#addrspacec)).
In particolare, ogni volta che tale funzione viene chiamata, essa verificherà che la pagina cercata si trovi nella Page Table tramite la funzione `pt_get_paddr()`. Se presente, allora essa fornirà l'indirizzo fisico. Altrimenti, verrà controllato se la pagina sitrova nello *swapfile* tramite la funzione `swapfile_swapin()` (definita in [swapfile.c](#swapfilec)) che provvederà a fare lo swap-in di tale pagina e a fornirne l'indirizzo fisico. In fine, se non dovesse trovarsi nemmeno dentro lo swapfile, la pagina dovrà essere caricata dall'ELF file e avrà inizio il processo di gestione del *Page Replacement* utilizzando le funzioni `vm_fault_page_replacement_[code] [data] [stack]()` (definite in [addrspace.c](#addrspacec)) per i relativi segmenti di codice, dato e stack.
Per concludere, la entry della pagina verrà inserita nella TLB tramite la funzione `vmtlb_write()`(definita in [vm_tlb.c](#vm_tlbc)).

## Altri dettagli implementativi

### coremap.c

Qui è presente ciò che occorre per tenere traccia dei frame fisici e quindi gestire la memoria del kernel attraverso una *coremap*. Questa viene inizializzata attraverso la funzione `coremap_init()`. Essa, in particolare, alloca e inizializza due vettori di dimensione `nRamFrames` i.e., la dimensione che al boot viene assegnata alla RAM, e tali vettori saranno `freeRamFrames[]` e `allocSize[]` i quali, rispettivamente, rappresenteranno il vettore le cui entry rappresenteranno le locazioni di memoria occupate (entry a 1) e libere (entry a 0) e l'altro sarà invece il vettore che terrà traccia della dimensione allocata alle varie pagine.

Proseguendo, troviamo una funzione di supporto, la  `getfreeppages()`. Essa viene utilizzata per cercare un insieme lungo `npages` di pagine consecutive libere operando in mutua esclusione sul vettore `freeRamFrames[]` e, in caso di successo, ritornerà l'indirizzo fisico del primo frame libero. 
Poi troviamo la funzione `coremap_getppages()` che in realtà è un wrapper alla `getfreeppages()`.

In fine, troviamo la funzione `coremap_freepages()`. Essa, passatole come parametro l'indirizzo fisico `addr`, libererà le pagine precedentemente allocate e lo farà a paratire da quell'indirizzo, recuperando il numero di pagine da liberare dal vettore `allocSize[]`.

### addrspace.c

Qui è dove viene gestita l'implementazione della Virtual Memory. Lo spazio di indirizzamento di un programma in OS/161 può essere rappresentato come l'insieme di tre segmenti: *codice*, *dato* e *stack*. In particolare, abbiamo optato per la creazione di una `struct addrspace{}` (inizializzata tramite la funzione `as_create()`) che contenesse le informazioni riguardanti i tre segmenti:

```C
/* in kern/include/addrspace.h */

struct addrspace {
        vaddr_t as_vbase1;  /* base virtual address of code segment */
        size_t as_npages1;  /* size (in pages) of code segment */
        vaddr_t as_vbase2;  /* base virtual address of data segment */
        size_t as_npages2;  /* size (in pages) of data segment */
        paddr_t as_stackpbase;  /* base physical address of stack */

        // Additional data:
        off_t code_offset;  /* code offset within elf file */
        uint32_t code_size;
        off_t data_offset; /* data offset within elf file */
        uint32_t data_size;
        struct vnode *v; /* file descriptor */

        int count_proc;  /* pages counter for this process loaded into the Page Table */
};
```

Innanzitutto, in questo file C, è presente la funzione `vm_bootstrap()` che viene viene chiamata, per l'appunto, al bootstrap del sistema e ha il compito di inizializzare tutto ciò che riguarda il Virtual Memory System i.e., la coremap, la Page Table, lo swapfile e la tlb.

Precedentemente, parlando del [TLB fault](#flusso-del-caricamento-di-una-pagina-dopo-un-tlb-fault), abbiamo discusso la funzione `vm_fault()` qui implementata e le relative funzioni `vm_fault_page_replacement_code()`, `vm_fault_page_replacement_data()` e `vm_fault_page_replacement_stack()`. Queste tre funzioni sono molto simili tra loro e, inanzitutto, dopo aver controllato che non si sfori il range di indirizzi appartenente al relativo segmento, esse avviano l'inserimento delle pagine del segmento del processo, in particolare:
* Se il processo ha già un numero di pagine in memoria >= al numero massimo consentito i.e., 32, allora sarà una pagina dello stesso processo ad essere cercata e rimpiazzata combinando delle primitive messe a disposizione da [pt.c](#ptc) e [swapfile.c](#swapfilec). 
* Altrimenti, tramite la `coremap_getppages()` (definita in [coremap.c](#coremapc)) si ricerca un frame libero. Se questo frame non sarà disponibile allora occorrera fare lo swap con una pagina appartentente a un qualsiasi altro processo seguendo una strategia FIFO.

Successivamente la Page Table verrà aggiornata e, nel caso in cui il fault fosse partito da un segmento di codice o di dati, dall'ELF verrà caricata la relativa pagina tramite la `load_page_from_elf()` (definita in [loadelf.c](#loadelfc)).

Altre funzioni degne di nota sono:
* La `as_destroy()`: si occupa di rimuovere tutte le entries della Page Table di un determinato processo in fase di distruzione, chiude l'ELF file e finalmente libera lo spazio dell'addresspace.
* La `as_activate()`: attiva l'addresspace del processo corrente, invalidando le entry della TLB appartenenti ad altri processi.
* La `as_define_region()`: definisce una regione dell'addresspace settando l'indirizzo base, il numero di pagine, la dimensione e il vnode.
 
### pt.c

In questo file è presente il codice necessario per gestire l’Inverted Page Table, che tiene traccia per ogni frame fisico della RAM, della pagina virtuale in esso allocata. Tramite la `pt_init()` verranno inizializzate le strutture di supporto. La prima operazione è assegnare la dimensione alla variabile `nRamFrames`, dato che la dimensione della RAM viene letta solo all’accensione del sistema. In seguito viene allocato dinamicamente e inizializzato il vettore `ipt[]` con una dimensione di `nRamFrames`. 

```C
struct ipt_t {
    pid_t pid;
    vaddr_t vaddr;
    bool invalid;
    unsigned char flags;
    unsigned int counter;
};
```

Le funzioni per la sua gestione sono: 
+ `pt_add_entry()` che viene utilizzata dalla `vm_fault_page_replacement_[code][data][stack]()` dopo aver trovato un frame libero o da rimpiazzare per caricare una pagina dal backing store. 
+ `pt_replace_entry()` e `pt_replace_any_entry()` sono chiamate dalla `vm_fault_page_replacement_[code][data][stack]()` rispettivamente quando il processo ha già un massimo di pagine ad esso allocato e quando non sono presenti frame liberi in memoria. Nel primo caso la pagina da rimpiazzare dovrà appartenere al processo stesso. La strategia di rimpiazzamento è First In First Out. Infatti ad ogni entry della page table viene assegnato un valore `ipt[i] -> counter`, che viene usato per tenere traccia delle pagine da più tempo in RAM. La pagine con valore maggiore viene rimpiazzata. 
Ogni volta che viene aggiunta una pagina con `pt_add_entry()` si fa una chiamata a `upgrade_counter()`, che incrementa questo valore in ogni entry valida della tabella, mentre la nuova pagina viene inserita con valore 0.
+ `pt get_paddr()` viene usata dalla `vm_fault()` per sapere se, dato il pid e il virtual address, la pagina richiesta si trova in memoria e, se c’è, ne restituisce l’indirizzo fisico. 
+ `pt_remove_entry()` e `pt_remove_entries()` servono per invalidare una o più entry della tabella. La seconda in particolare, viene chiamata alla chiusura di un processo dalla `as_destroy()` per invalidare tutti i frame appartenenti ad esso, in modo da poterli riutilizzare.
+ `pt_destroy()` libera la page table attraverso la `kfree()`
+ `pt_getFlagsByIndex()` ` pt_getVaddrByIndex()` ` pt_getPidByIndex()` `pt_setFlagsAtIndex()` sono funzioni che vengono utilizzate all’occorrenza per ottenere o settare i valori delle entry

### swapfile.c

In questo file sono state implementate le funzioni per gestire il file di swap, file in cui vengono salvate le informazioni sulle pagine che vengono spostate dalla RAM al disco in caso di page replacement. In questo modo sono più velocemente accessibili se nuovamente richieste.

```C
typedef struct {
    vaddr_t v_pages; 
    pid_t pid; 
    unsigned char flags;
} swapfile;
```
Lo scheletro della struttura utilizzata è quello sopra riportato. Il vettore `swapfile[]` viene allocato e inizializzato nel momento in cui viene fatto il bootstrap della memoria virtuale con una chiamata a `swapfile_init()`. La dimensione è pari a `SWAP_SIZE` definita in `vm.h`. 
Le operazioni principali per la sua gestione sono `swapfile_swapin()` e `swapfile_swapout()`.

`swapfile_swapin()` viene chiamata da `vm_fault()` in caso di page table miss, cosa che richiede una sostituzione di pagina. Il suo compito è cercare se la pagina richiesta si trova nel file. In caso affermativo deve caricarla effettuando lo swapout di un'altra pagina, trovata chiamando `pt_replace_entry()` e `swapfile_swapout()`. In seguito carica la pagina con `uio_kinit()` ed inseriscela nuova entry in page table con `pt_add_entry()`. 

`swapfile_swapout()` viene chiamata sia dalle funzioni `vm_fault_page_replacement_[code][data][stack]()` che da `swapfile_swapin()`. Le prime in caso la pagina richiesta non si trovi neanche nello swapfile, la seconda nel caso opposto. Il suo compito è cercare un frame libero nello swapfile, riportare la pagina da rimpiazzare in backing store e invalidando l'eventuale entry nella tlb con `vmtlb_clean()`  e quella in pagetable con `pt_remove_entry()`.

### vm_tlb.c

In questo file C è presente il codice necessario per la manipolazione della TLB. Essa è stata rappresentata attraverso la seguente struttura dati:
```c
static struct tlb_map_t tlb_map;

typedef struct tlb_map_t {
    unsigned char *map;  /* bitmap: ogni bit identificherà una cella della TLB */
    unsigned char size;  /* dimensione del vettore map */
} tlb_map_t;
```
Tale struttura verrà allocata attraverso la funzione `tlbmap_init()` e inizializzata attraverso la funzione `vmtlb_init()`.

La funzione `tlb_get_rr_victim()`, utilizzando un algoritmo basato sul Round Robin, si occupa della selezione di una vittima da rimpiazzare all'interno della TLB.

La funzione `vmtlb_searchIndex()` si occupa di scandire in mutua esclusione la TLB alla ricerca di un frame libero. Ciò lo fa sfruttando un doppio ciclo: un ciclo esterno per ogni entry del vettore `tlb_map.map[]` e uno interno su ogni bit di ogni entry del vettore. Una volta trovato un bit libero della TLB, ne ritornerà il suo indice, altrimenti ritornerà `-1`.
Tale funzione, verrà utilizzata dalla `vmtlb_write()`. Quest'ultima, a sua volta, viene chiamata in [addrspace.c](#addrspacec) dalla `vm_fault()` per inserire una nuova entry nella TLB dopo un fault. Infatti, essa cercherà una entry vuota tramite la `vmtlb_searchIndex()` e se anche questa non dovesse trovare una entry libera, allora selezionerà una vittima da sostituire tramite la funzione `tlb_get_rr_victim()`. Infine,una volta trovato un indice, aggiungerà la nuova entry chiamando la funzione MIPS `tlb_write()`.

Infine, troviamo la funzione `vmtlb_clean()` che, dato l'indice, ne invaliderà la entry corrispondente. Essa sarà usata dalle funzioni `swapfile_swapout()` (in [swapfile.c](#swapfilec)) e `as_activate()` (in [addrspace.c](#addrspacec)).

### vmstats.c

In questo file sono presenti le funzioni per l'inizializzazioine, l'incremento e la stampa delle statistiche. Queste sono raccolte in una struttura definita in `vmstats.h`. 

+ `tlb_fault`: numero di TLB misses. Incrementato in `vm_fault()`.
+ `tlb_faultFree`: numero di TLB miss per cui c'era spazio per una nuova TLB entry. Incrementato in `vmtlb_write()`.
+ `tlb_faultReplacement`: numero di TLB miss che richiedono un rimpiazzo di una entry. Viene incrementato in `vmtlb_write()`.
+ `tlb_invalidation`: numero di volte in cui la TLB è stata invalidata. Incrementato quando un nuovo processo viene attivato e le entry relative al vecchio processo devono essere invalidate in `as_activate()`
+ `tlb_reload`: numero di volte in cui la pagina è stata trovata nella page table dopo un TLB miss. Incrementato in `vm_fault()` se la pagina è stata trovata da `pt_get_paddr()` 
+ `pf_zero`: numero di TLB miss che richiedono che una pagina venga azzerata. Incrementato in `vm_fault_page_replacement_stack()` poichè lo stack necessita una pagina vuota.
+ `pf_disk`: numero di page fault che richiedono che una pagina venga caricata dal disco Incrementato in `vm_fault_page_replacement_[code][data]()` dopo aver caricato una pagina dall'ELF file e in `vm_fault()` dopo aver caricato la pagina con `swapfile_swapin()`.
+ `pf_elf`: numero di page fault che richiedono che una pagina venga caricata dall'ELF file. Incrementato in `vm_fault_page_replacement_[code][data]()`.
+ `pf_swapin`: numero di page fault che richiedono una pagina dallo swapfile. Incrementato in `swapfile_swapin()`.
+ `pf_swapout`: numero di pagine che richiedono che una pagina venga scritta nello swapfile. Incrementato in `swapfile_swapout()`.

## Statistiche

|    Test    | TLB Faults | TLB Faults with Free | TLB Faults with Replace | TLB Invalids | TLB Reloads | Page Faults (zero filled) | Page Faults (disk) | Page Faults from ELF | Swapin | Swapout |
| :--------: | :--------: | :------------------: | :---------------------: | :----------: | :---------: | :-----------------------: | :----------------: | :-----------: | :-----------: | :-------------: |
|   palin    |     5      |          5           |            0            |      7       |      0      |             1            |         4          |       4      |       0       |        0        |
|    sort    |     2771      |          2771           |            0            |      7      |      0      |            1            |         2770          |       291       |       2479       |        2740        |
|    huge    |     3688      |          3688           |            0            |      6       |      0      |             1             |         3687          |       514       |       3173       |        3657        |
|  matmult   |     906      |          906           |            0            |      6       |      0      |             1             |         905          |       382       |       523       |        875        |
|   ctest    |     127298      |          127298           |            0            |      7       |      0      |             1             |         127297          |       259       |       127038       |        127267        |
|    zero    |     4      |          4           |            0            |      6       |      0      |             1             |         3          |       3       |       0       |        0        |

Come si può notare nella tabella sovrstante, non avvengono TLB faults con replacement perchè, assegnate la dimensione della TLB e il numero limitato di pagine allocabili per processo, non ci sarà bisogno di sostituzione. Infatti quando avvengono dei miss, è perchè la pagina non è effettivamente in memoria e deve quindi essere caricata. Lo stesso comportamento influenza anche il valore di TLB reload, perchè, se la pagina è in RAM, allora sarà già presente una entry in TLB. Per quanto riguarda i page fault che richiedono una pagina vuota, si può affermare che la pagina di stack necessaria è una perchè i test effettuati non richiedono ulteriore spazio nello stack.
