#include "../winix.h"

void syscall_printf(proc_t *who, message_t *m){
    void *ptr = NULL, *ptr2 = NULL;
    ptr = get_physical_addr(m->p1,who);
    ptr2 = get_physical_addr(m->p2,who);
    kprintf_vm(ptr,ptr2,who->rbase);
    
    m->i1 = 0;
    winix_send(who->pid,m);
}