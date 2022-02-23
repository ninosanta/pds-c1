/**
 * @file coremap.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2022-02-23
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef _COREMAP_H_
#define _COREMAP_H_


#define COREMAP_INIT_SUCCESS 0

int coremap_init(void);
int coremap_isTableActive(void);
paddr_t coremap_getppages(unsigned long npages);
int coremap_freepages(paddr_t addr);

#endif