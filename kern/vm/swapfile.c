/**
 * @file swapfile.c
 * @author your name (you@domain.com)
 * @brief code for managing and manipulating the swapfile
 * @version 0.1
 * @date 2021-12-15
 *
 * @copyright Copyright (c) 2021
 *
 */

// Librerie
#include <types.h>
#include <kern/errno.h>
#include <lib.h>
#include <spl.h>
#include <cpu.h>
#include <spinlock.h>
#include <proc.h>
#include <current.h>
#include <mips/tlb.h>

#include <vm.h>
#include <uio.h>
#include <vnode.h>
#include <elf.h>
#include <vfs.h>
#include <kern/fcntl.h>

#include "swapfile.h"
#include "vmstats.h"
#include "vm_tlb.h"
#include "coremap.h"
#include "pt.h"

// Variabili globali
static swapfile *sw;
static struct spinlock swapfile_lock;
unsigned int sw_length;
struct vnode *swapstore;
int fd;
static const char swapfilename[] = "emu0:SWAPFILE";

/************************************************************
 *                                                          *
 * Implementazione delle funzioni                           *
 *                                                          *
 ************************************************************/

/**
 * @brief Iizializzazione dello swapfile
 * 
 * @param length 
 * @return int 
 */
int swapfile_init(long length)
{
    int i;
    char path[sizeof(swapfilename)];

    sw = (swapfile *)kmalloc(sizeof(swapfile) * length);
    if (sw == NULL)
    {
        return 1;
    }

    for (i = 0; i < length; i++)
    {
        sw[i].flags = 0;
        sw[i].pid = -1;
        sw[i].v_pages = 0x0;
    }

    sw_length = length;
    strcpy(path, swapfilename);
    fd = vfs_open(path, O_RDWR | O_CREAT, 0, &swapstore);
    if (fd)
    {
        kprintf("swap: error %d opening swapfile %s\n", fd, swapfilename);
        kprintf("swap: Please create swapfile/swapdisk\n");
        panic("swapfile_init can't open swapfile");
    }
    spinlock_init(&swapfile_lock);

    return SWAPMAP_INIT_SUCCESS;
}

/**
 * @brief Cerca se la pagina passata come parametro si trova nel file, in modo da ottenere il suo indirizzo nel backing store
 *        Prende una pagina del backing store e fa swap con una nella page table
 * 
 * @param vaddr 
 * @param paddr 
 * @param pid 
 * @param as 
 * @return int 
 */
int swapfile_swapin(vaddr_t vaddr, paddr_t *paddr, pid_t pid, struct addrspace *as)
{
    unsigned int i;
    int indexR;
    int res;
    struct iovec iov;
    struct uio ku;

    // Scansione del vettore di pagine presenti nello backing store
    for (i = 0; i < sw_length; i++)
    {

        // Indirizzo di pagina del processo PID trovato
        if (sw[i].v_pages == vaddr && sw[i].pid == pid)
        {
            as->count_proc++;
            // Verifica se l'aggiunta della pagina comporta il superamento della soglia massima in pt
            if (as->count_proc >= MAX_PROC_PT)
            {
                indexR = pt_replace_entry(pid);
                swapfile_swapout(pt_getVaddrByIndex(indexR), indexR * PAGE_SIZE, pid, pt_getFlagsByIndex(indexR));
                as->count_proc--;
                *paddr = indexR * PAGE_SIZE; 
            }
            else
            {
                // Richiedi una pagina fisica libera alla coremap
                *paddr = coremap_getppages(1);
                if (*paddr == 0)
                { // Non ci sono pagine libere nel vettore corempa_allocSize
                    indexR = pt_replace_any_entry();
                    swapfile_swapout(pt_getVaddrByIndex(indexR), indexR * PAGE_SIZE, pid, pt_getFlagsByIndex(indexR));
                    as->count_proc--;
                    *paddr = indexR * PAGE_SIZE;
                }
            }
            // Pulisce la pagina appena presa
            as_zero_region(*paddr, 1); // Inizilizza a 0 la pagina
            vmstats_report_pf_zero_increment();

            // perform the I/O
            uio_kinit(&iov, &ku, (void *)PADDR_TO_KVADDR(*paddr), PAGE_SIZE, i * PAGE_SIZE, UIO_READ);
            res = VOP_READ(swapstore, &ku);
            if (res)
            {
                panic("something went wrong while reading from the swapfile");
            }

            if (ku.uio_resid != 0)
            {
                /* short read; problem with executable? */
                kprintf("ELF: short read on header - file truncated?\n");
                /* return ENOEXEC; */
            }

            // pid uguale a -1 significa che il blocco referenziato nello swapfile può essere riusato
            sw[i].pid = -1;
            vmstats_report_pf_swapin_increment(); // Incremento numero di swapin effettuate
            //Aggiungo la nuova pagine nella page table
            pt_add_entry(vaddr, *paddr, pid, sw[*paddr / PAGE_SIZE].flags);
            return SWAPMAP_SUCCESS;
        }
    }
    return SWAPMAP_REJECT;
}

/**
 * @brief Prende una pagine e la rimette nel backing store, salvando il suo indirizzo nel file di swap
 * 
 * @param vaddr 
 * @param paddr 
 * @param pid 
 * @param flags 
 * @return int 
 */
int swapfile_swapout(vaddr_t vaddr, paddr_t paddr, pid_t pid, unsigned char flags)
{

    unsigned int frame_index = 0, i, err;
    struct iovec iov;
    struct uio ku;

    if (vaddr > MIPS_KSEG0)
        return -1;

    // Cerco il primo frame libero in cui poter fare swapout
    spinlock_acquire(&swapfile_lock);
    for (i = 0; i < sw_length; i++)
    {
        if (sw[i].pid == -1)
        {
            frame_index = i;
            break;
        }
    }
    spinlock_release(&swapfile_lock);

    if (i == sw_length)
        panic("Out of swap space");

    // Faccio SWAPOUT
    uio_kinit(&iov, &ku, (void *)PADDR_TO_KVADDR(paddr), PAGE_SIZE, frame_index * PAGE_SIZE, UIO_WRITE);
    err = VOP_WRITE(swapstore, &ku);
    if (err)
    {
        panic(": Write error: %s\n", strerror(err));
    }

    // Inizializzazione della struttura dati
    spinlock_acquire(&swapfile_lock);
    sw[frame_index].v_pages = vaddr;
    sw[frame_index].flags = flags & 0x01;
    sw[frame_index].pid = pid;
    spinlock_release(&swapfile_lock);

    // Il rapporto tra tlb_page(1kb) e backing store_page (4kb)è di un fattore 4
    // Pertando data il numero di pagina associato alla tlb occorre dividere per 4 così da
    // associarlo al numero di pagina in backing store
    vmtlb_clean(flags >> 2); // 0x[ 000000 ] [ 0 ] [ 0 ]

    pt_remove_entry(paddr / PAGE_SIZE);
    vmstats_report_pf_swapout_increment();
    return 1;
}
