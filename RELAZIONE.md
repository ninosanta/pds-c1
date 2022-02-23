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

Questo file è composto da un unica funzione, la `runprogram()`, la quale riceve come unico parametro il nome del programma. Tale programma verrà aperto in sola lettura dalla `vfs_open()` e un nuovo address space verrà creato e attivato per esso, rispettivamente tramite le funzioni `as_create()` e `as_activate()` (definite in [addrspace.c](#addrspacec)). Dopodiché, tramite la funzione `load_elf()` (definita in [loadelf.c](#loadelfc)), vengono definiti i segmenti di dato e utente del programma e ne viene caricato l'eseguibile nell'address space corrente settandone anche l'entrypoint del programma. In fine, lo stack segment verrà definito tramite la funzione `as_define_stack()` e finalmente il processo potrà partire tramite la funzione `enter_new_process()`.

Rispetto al vecchio `runprogram.c` la differenza sostanziale è che qui non chiudiamo il file ELF perché le pagine verranno caricate su richiesta. Quindi, il file ELF verrà chiuso solamente quando il programma avrà terminato l'esecuzione.

### loadelf.c

Le funzione `load_elf()` in precedenza si occupava del caricamento dell'intero file in memoria ma, seguendo la politica della paginazione su richiesta, non occorre più sia così.

Inoltre, non è previsto l'utilizzo delle funzione `load_segment()` e tutto il necessario sarà fatto nella `load_elf()` che, come già detto, ritornerà l'entrypoint del processo che verrà utilizzato per avviare il processo. Qui, inoltre, verrà scandito l'header dell'eseguibile e verranno definite le regioni dell'addess space tramite la funzione `as_define_region()` preparandole tramite la funzione `as_prepare_load()` (definite in [addrspace.c](#addrspacec)). 

## Flusso del caricamento di una pagina dopo un TLB fault

In caso di TLB miss, viene generato un Page Fault i.e., un'eccezione di indirizzo che indica la mancanza della pagina richiesta nella TLB. E questa eccezione viene gestita attraverso la funzione `vm_fault()` (definita in [addrspace.c](#addrspacec)).
In particolare, ogni volta che tale funzione viene chiamata, essa verificherà che la pagina cercata si trovi nella Page Table tramite la funzione `pt_get_paddr()`. Se sì, allora essa fornirà l'indirizzo fisico. Altrimenti, verrà controllato se la pagina è presente nello *swapfile* tramite la funzione `swapfile_swapin()` (definita in [swapfile.c](#swapfilec)) che provvederà a fare lo swap-in di tale pagina e a fornirne l'indirizzo fisico. In fine, se non dovesse trovarsi nemmeno dentro lo swapfile, la pagina dovrà essere caricata dal disco e avrà inizio il processo di gestione del *Page Replacement* utilizzando le funzioni `vm_fault_page_replacement_[code] [data] [stack]()` (definite in [addrspace.c](#addrspacec)) per i relativi segmenti di codice, dato e stack.
Per concludere, la entry verrà inserita nella TLB tramite la funzione `vmtlb_write()`(definita in [vm_tlb.c](#vm_tlbc)).

## Altri dettagli implementativi

### coremap.c

Qui è presente ciò che occorre per tenere traccia dei frame fisici e quindi gestire la memoria del kernel attraverso una *coremap*. Questa viene inizializzata attraverso la funzione `coremap_init()`. Essa, in particolare, alloca e inizializza due vettori di dimensione `nRamFrames` i.e., la dimensione che al boot viene assegnata alla RAM, e tali vettori saranno `freeRamFrames[]` e `allocSize[]` i quali, rispettivamente, rappresenteranno il vettore le cui entry rappresenteranno le locazioni di memoria occupate (entry a 1) e libere (entry a 0) e l'altro sarà invece il vettore che terrà traccia della dimensione allocata alle varie pagine.

Proseguendo, troviamo una funzione di supporto, la  `getfreeppages()`. Essa viene utilizzata per cercare un insieme lungo `npages` di pagine consecutive libere operando in mutua esclusione sul vettore `freeRamFrames[]` e, in caso di successo, ritornerà l'indirizzo fisico del primo frame libero. 
Poi troviamo la funzione `coremap_getppages()` che in realtà è un wrapper alla `getfreeppages()`.

In fine, troviamo la funzione `coremap_freepages()`. Essa, passatole come parametro l'indirizzo fisico `addr`, libererà le pagine precedentemente allocate e lo farà a paratire da quell'indirizzo, recuperando il numero di pagine da liberare dal vettore `allocSize[]`.

### addrspace.c
TODO
`vm_fault()` già discussa [in precedenza](#flusso-del-caricamento-di-una-pagina-dopo-un-tlb-fault).

### pt.c

### swapfile.c

### vm_tlb.c

### vmstats.c

## Statistiche

|    Test    | TLB Faults | TLB Faults with Free | TLB Faults with Replace | TLB Invalids | TLB Reloads | Page Faults (zero filled) | Page Faults (disk) | ELF File Read | Swapfile Read | Swapfile Writes |
| :--------: | :--------: | :------------------: | :---------------------: | :----------: | :---------: | :-----------------------: | :----------------: | :-----------: | :-----------: | :-------------: |
|   palin    |     x      |          x           |            x            |      x       |      x      |             x             |         x          |       x       |       x       |        x        |
|    sort    |     x      |          x           |            x            |      x       |      x      |             x             |         x          |       x       |       x       |        x        |
|    huge    |     x      |          x           |            x            |      x       |      x      |             x             |         x          |       x       |       x       |        x        |
|  matmult   |     x      |          x           |            x            |      x       |      x      |             x             |         x          |       x       |       x       |        x        |
|   ctest    |     x      |          x           |            x            |      x       |      x      |             x             |         x          |       x       |       x       |        x        |
|    zero    |     x      |          x           |            x            |      x       |      x      |             x             |         x          |       x       |       x       |        x        |
| tlbreplace |     x      |          x           |            x            |      x       |      x      |             x             |         x          |       x       |       x       |        x        |