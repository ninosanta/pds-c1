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
#include "swapfile.h"


// Define

// Variabili globali

/************************************************************
 *                                                          *
 * Implementazione delle funzioni                           *
 *                                                          *
 ************************************************************/
int swapfile_init(void) {

    return SWAPMAP_INIT_SUCCESS;
}

void swapfile_swapin(void){

}

void swapfile_swapout(void){

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



