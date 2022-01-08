/**
 * @file swapfile.c
 * @author your name (you@domain.com)
 * @brief 
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

// Define

// Variabili globali
static swapfile *sw; 
static struct spinlock swapfile_lock;
unsigned int sw_length;
struct vnode* swapstore;
int fd;

tlb_report vmstats_report;

static const char swapfilename[] = "emu0:SWAPFILE";

/************************************************************
 *                                                          *
 * Implementazione delle funzioni                           *
 *                                                          *
 ************************************************************/
int swapfile_init(int length) {
    int i;
    char path[sizeof(swapfilename)];

    sw = (swapfile *) kmalloc(sizeof( swapfile)*length);
    if(sw==NULL){
        return 1;
    }
    
    for(i=0;i<length;i++){
        sw[i].flags = 0;
	    sw[i].pid = -1;
	    sw[i].v_pages = 0x0;
    }
    
    sw_length = length;
    strcpy(path, swapfilename);
    fd = vfs_open(path, O_RDWR | O_CREAT ,0, &swapstore);
    if (fd){
        kprintf("swap: error %d opening swapfile %s\n", fd, swapfilename);
        kprintf("swap: Please create swapfile/swapdisk\n");
        panic("swapfile_init can't open swapfile");
    }
    spinlock_init(&swapfile_lock);

    vmstats_report.pf_swapin = 0;
    vmstats_report.pf_swapout = 0;

    return SWAPMAP_INIT_SUCCESS;
}

int swapfile_swapin(vaddr_t vaddr, paddr_t *paddr, pid_t pid, struct addrspace *as){
    // unsigned int i;
    // int indexR;
    // int res;
    // struct iovec iov;
    // struct uio ku;

    // for(i=0;i<sw_length;i++){
    //     if(sw[i].v_pages==vaddr && sw[i].pid==pid){
    //         as->count_proc++;
    //         if (as->count_proc>=MAX_PROC_PT){
    //             indexR = pagetable_replacement(pid);
    //             swapfile_swapout(pagetable_getVaddrByIndex(indexR), indexR*PAGE_SIZE, pid, pagetable_getFlagsByIndex(indexR));
    //             as->count_proc--;
    //             *paddr = indexR*PAGE_SIZE;
    //         }
    //         else {
    //             *paddr = getppages(1);
    //             if (*paddr==0){
    //                 indexR = pagetable_replacement(pid);
    //                 swapfile_swapout(pagetable_getVaddrByIndex(indexR), indexR*PAGE_SIZE, pid, pagetable_getFlagsByIndex(indexR));
    //                 as->count_proc--;
    //                 *paddr = indexR*PAGE_SIZE;
    //             }
    //         }
    //         // clean the page just got by allocation (or previously swapped)
    //         pf_z++;
    //         as_zero_region(*paddr, 1);
    //         // perform the I/O
    //         uio_kinit(&iov, &ku, (void *)PADDR_TO_KVADDR(*paddr), PAGE_SIZE, i*PAGE_SIZE, UIO_READ);
    //         res = VOP_READ(swapstore, &ku);
    //         if (res) {
    //             panic("something went wrong while reading from the swapfile");
    //         }

    //         if (ku.uio_resid!=0){
    //             /* short read; problem with executable? */
	// 	        kprintf("ELF: short read on header - file truncated?\n");
    //             /* return ENOEXEC; */
    //         }
    //         // pid equals to -1 means that the referenced block in the swapfile can be now reused
    //         sw[i].pid = -1;
    //         // add the recently swapped-in page in the IPT
    //         pf_sw_in++;
    //         pagetable_addentry(vaddr, *paddr, pid, sw[*paddr/PAGE_SIZE].flags);
    //         return 1;
    //     }
    // }

    (void)vaddr;
    (void)paddr;
    (void)pid;
    (void)as;

    return 0;


}

int swapfile_swapout(vaddr_t vaddr, paddr_t paddr, pid_t pid, unsigned char flags){

//  unsigned int frame_index, i, err;
//     struct iovec iov;
//     struct uio ku;

//     if (vaddr > MIPS_KSEG0)
//         return -1;

//     //CERCO IL PRIMO FRAME LIBERO IN CUI POTER FARE SWAPOUT
//     spinlock_acquire(&swapfile_lock);
//     for (i = 0; i < sw_length; i++)
//     {
//         if (sw[i].pid == -1)
//         {
//             frame_index = i;
//             break;
//         }
//     }
//     spinlock_release(&swapfile_lock);

//     if (i == sw_length)
//         panic("Out of swap space");

//     //FACCIO SWAPOUT
//     uio_kinit(&iov, &ku, (void *)PADDR_TO_KVADDR(paddr), PAGE_SIZE, frame_index * PAGE_SIZE, UIO_WRITE);
//     err = VOP_WRITE(swapstore, &ku);
//     if (err)
//     {
//         panic(": Write error: %s\n", strerror(err));
//     }

//     spinlock_acquire(&swapfile_lock);
//     sw[frame_index].v_pages = vaddr;
//     sw[frame_index].flags = flags & 0x01;
//     sw[frame_index].pid = pid;
//     spinlock_release(&swapfile_lock);

//     tlb_clean_entry(flags >> 2);
//     pagetable_remove_entry(paddr / PAGE_SIZE);
//     pf_sw_out++;
//     return 1;

    (void)vaddr;
    (void)paddr;
    (void)pid;
    (void)flags;
    return 0;
}





// swapfile.c: code for managing and manipulating the swapfile

//vedi CHAPTER 9 --> pagina 55



// A process can be swapped temporaly out of memory
// to a backing store an then brought back into memory 
// for continued execution

// swapping is normally disabled, starts whan a threshold of allocated memory is reached 
//disabled when you're back under the threshold


//nello swap file ci devono essere scritte le pagine che devono essere scritte su disco 
//la politica di rimpiazzamnto è a nostra scelta



