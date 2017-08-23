/**
 * 
 * Exception-handling routines for WINIX.
 *
 * @author Bruce Tan
 * @email brucetansh@gmail.com
 * @author Paul Monigatti
 * @email paulmoni@waikato.ac.nz
 * @create date 2016-09-19
 * @modify date 2017-08-23 06:05:21
*/

#ifndef _K_EXCEPTION_H_
#define _K_EXCEPTION_H_ 1

//System uptime, in ticks.
extern int system_uptime;

typedef void (*expt_handler_t)( void );

/**
 * Initialises Exception Handlers and configures timer.
 **/
void init_exceptions();
int irq_count();
int* get_exception_stack_top();
int* get_exception_stack_bottom();

#define in_interrupt()  (irq_count())
#define EXCEPTION_STACK_SIZE    300

#endif
