// swapfile.c: code for managing and manipulating the swapfile
#include <swapfile.h>

//vedi CHAPTER 9 --> pagina 55



// A process can be swapped temporaly out of memory
// to a backing store an then brought back into memory 
// for continued execution

// swapping is normally disabled, starts whan a threshold of allocated memory is reached 
//disabled when you're back under the threshold


//nello swap file ci devono essere scritte le pagine che devono essere scritte su disco 
//la politica di rimpiazzamnto è a nostra scelta

unsigned int swapfile_init( unsigned int swapfile_dim) {
    //creare
    //deve avere una dimensione definita
    return swapfile_dim++; //messo per evitare warning
}

void swapfile_swap_out(void){}

void swapfile_swap_in(void){}


