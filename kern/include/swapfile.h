#ifndef _SWAPFILE_H_
#define _SWAPFILE_H_

#define SWAPMAP_INIT_SUCCESS 0

int swapfile_init(void);
void swapfile_swapin(void);
void swapfile_swapout(void);

#endif
