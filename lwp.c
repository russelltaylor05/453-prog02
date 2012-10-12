#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/wait.h>

#include "lwp.h"

void print_table();
int roundrobin();

/* global variable to define lwp_table[PROC_LIMIT]) */

/* main stack pointer to return to after lwp_stop() */
unsigned long *initial; 

/* scheduler function */
schedfun scheduler;

/*
 * Start the LWP threads
 */
void lwp_start()
{  
  SAVE_STATE();
  GetSP(initial);
  SetSP(lwp_ptable[0].sp);
  lwp_running = 0;
  RESTORE_STATE();
}

void lwp_stop()
{  
  SAVE_STATE();
  GetSP(lwp_ptable[lwp_running].sp);

  SetSP(initial);
  RESTORE_STATE();
}
    

void lwp_yield()
{

  SAVE_STATE();
  GetSP(lwp_ptable[lwp_running].sp);
    
  lwp_running = (lwp_running + 1) % lwp_procs;
  
  SetSP(lwp_ptable[lwp_running].sp);
  RESTORE_STATE();  
}


int new_lwp(lwpfun funct, void *ptr, size_t size)
{
  unsigned long *sp;
  int i;
  i = 0;

  lwp_ptable[lwp_procs].pid = lwp_procs+1;
  lwp_ptable[lwp_procs].stack = malloc(size*4);
  lwp_ptable[lwp_procs].stacksize = size;
  lwp_ptable[lwp_procs].sp = lwp_ptable[lwp_procs].stack + size;
    
  /* add params to stack*/
  lwp_ptable[lwp_procs].sp--;
  *(lwp_ptable[lwp_procs].sp) = (unsigned long)ptr;
    
  /* add return value to stack */
  lwp_ptable[lwp_procs].sp--;
  *(lwp_ptable[lwp_procs].sp) = (unsigned long)lwp_exit;

      
  /* pointer to func */  
  lwp_ptable[lwp_procs].sp--;
  *(lwp_ptable[lwp_procs].sp) = (unsigned long)funct;
    
  /* EBP on stack */
  lwp_ptable[lwp_procs].sp--;
  *(lwp_ptable[lwp_procs].sp) = 0;
  sp = lwp_ptable[lwp_procs].sp;
  
  /* Make space for empty registers */
  lwp_ptable[lwp_procs].sp = lwp_ptable[lwp_procs].sp - 7;
  
  /* set EBP for register restory */
  *(lwp_ptable[lwp_procs].sp) = (unsigned long)sp;

  /* print stack */
  /*
  printf("stack top:\t %d\n", (int)(lwp_ptable[lwp_procs].stack + size));  
  printf("current:\t %d\n", (int)(lwp_ptable[lwp_procs].sp));  
  for(i = 0; i < 11; i++) {
    printf("Address: %d\t Value: %d \n", (int)(lwp_ptable[lwp_procs].sp + i), (int)*(lwp_ptable[lwp_procs].sp + i));
  }  
  printf("\n");
  */


  lwp_procs++;

  return lwp_ptable[lwp_procs-1].pid;

}

/*
 * Terminate current LWP
 */
void lwp_exit()
{
  int i;  
  unsigned long *sp;

  /* Should get onto a new stack before destroying the current one */
  
  free(lwp_ptable[lwp_running].stack);  
  lwp_procs--;
  
  if(lwp_procs == 0) {
    lwp_stop();
  }
  
  /* Move processes in table down */
  for (i = lwp_running; i < lwp_procs; i++) {
    lwp_ptable[i] = lwp_ptable[i+1];
  }
  
  /* Choose new process */
  sp = lwp_ptable[0].sp;
  SetSP(sp);
  RESTORE_STATE(); /* make last statement */

}

/*
void lwp_set_scheduler(schedfun sched) 
{
  if(sched) {
    scheduler = sched;
  } else {
    scheduler = roundrobin;
  }
  
}

int roundrobin()
{
  return (lwp_running + 1) % lwp_procs;
}
*/

/* 
 * Return PID of calling LWP
 */
int  lwp_getpid()
{
  return lwp_ptable[lwp_running].pid;
}


void print_table() 
{
  int i;
  printf("\n");
  printf("lwp_procs: %d\n", lwp_procs);
  for(i = 0; i < lwp_procs; i++) {
    printf("index: %d\t", i);
    printf("pid: %d\t", (int)lwp_ptable[i].pid);
    printf("stack: %d\t", (int)lwp_ptable[i].stack);
    printf("size: %d\t", (int)lwp_ptable[i].stacksize);
    printf("sp: %d\n", (int)lwp_ptable[i].sp);
  }

}
