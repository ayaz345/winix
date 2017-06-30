/**
 * System task for WINIX.
 *
 * Revision History:
 *  2016-09-19		Paul Monigatti			Original
 *  2016-11-20		Bruce Tan			Modified
 **/

#include "winix.h"
#include <sys/syscall.h>
#include <signal.h>

/**
 * Entry point for system task.
 **/
void system_main() {
	int counter = 0;
	FREE_MEM_END = 0x1ffff;
	
	//Print Memory Map
	kprintf("Text Segment: %x - %x\r\n", &TEXT_BEGIN, &TEXT_END);
	kprintf("Data Segment: %x - %x\r\n", &DATA_BEGIN, &DATA_END);
	kprintf("BSS Segment:  %x - %x\r\n", &BSS_BEGIN, &BSS_END);
	kprintf("Unallocated:  %x - %x\r\n", FREE_MEM_BEGIN, FREE_MEM_END);
	kprintf("%d kWords Free\r\n", ((unsigned long)(FREE_MEM_END - FREE_MEM_BEGIN)) / 1024);
	//Receive message, do work, repeat.
	while(1) {
		message_t m;
		int who;
		proc_t *caller, *pcurr;
		int response = 0;
		void *ptr = NULL, *ptr2 = NULL;

		//Get a message
		winix_receive(&m);
		who = m.src;
		caller = &proc_table[who];
		//kprintf("received from %s, call id %d, operation %d\n",p->name,p->proc_index,m.type );
		//Do the work
		switch(m.type) {

			//Gets the system uptime.
			case SYSCALL_GETC:
				m.i1 = kgetc();
				if(caller->state == RUNNABLE)
					winix_send(who,&m);
				break;

			case SYSCALL_UPTIME:
				m.i1 = system_uptime;
				winix_send(who, &m);
				break;

			//Exits the current process.
			case SYSCALL_EXIT:
				do_exit(caller,&m);
				break;

			case SYSCALL_PS:
				response = process_overview();
				break;

			case SYSCALL_FORK:
				pcurr = do_fork(caller);
				m.i1 = pcurr->proc_index;
				winix_send(who, &m);
				
				//send 0 to child
				m.i1 = 0;
				winix_send(pcurr->proc_index,&m);
				break;

			case SYSCALL_EXEC:
				response = exec_read_srec(get_proc(who));
				break;

			case SYSCALL_SBRK:
				m.p1 = do_sbrk(caller,m.i1);
				winix_send(who, &m);
				break;

			case SYSCALL_BRK:
				m.i1 = do_brk(caller,m.p1);
				winix_send(who, &m);
				break;

			case SYSCALL_PUTC:
				kputc(m.i1);
				break;

			case SYSCALL_PRINTF:
				//p1: str pointer
				//p2: args pointer
				ptr = get_physical_addr(m.p1,caller);
				ptr2 = get_physical_addr(m.p2,caller);
				kprintf_vm(ptr,ptr2,caller->rbase);
				// process_overview();
				break;

			case SYSCALL_ALARM:
				sys_alarm(caller,m.i1);
				break;

			case SYSCALL_SIGNAL:
				set_signal(caller,m.i1,m.s1);
				break;
				
			case SYSCALL_SIGRET:
				do_sigreturn(caller,m.i1);
				break;
			
			case SYSCALL_WAIT:
				do_wait(caller,&m);
				break;

			case SYSCALL_GETPID:
				m.i1 = who;
				winix_send(who,&m);
				break;
			
			default:
				kprintf("\r\n[SYSTEM] Process \"%s (%d)\" performed unknown system call %d\r\n", caller->name, caller->proc_index, m.type);
				end_process(caller);
				break;
		}
	}
}
