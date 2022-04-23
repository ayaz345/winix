#ifndef _KERNEL_ERROR_H_
#define _KERNEL_ERROR_H_

#include <errno.h>

#define MAX_ERR (PAGE_LEN)

#define IS_ERR(p)   ((unsigned long)(void *)(p) >= (unsigned long)-MAX_ERR)
#define ERR_PTR(p)  ((void *)p)
#define PTR_ERR(e)  ((unsigned long)e) 

#endif
