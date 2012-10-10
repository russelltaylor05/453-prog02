#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/wait.h>

#include "lwp.h"

void print_table();

/* global variable to define lwp_table[PROC_LIMIT]) */

/*
 * Yield control to another LWP
 */
void lwp_yield()
{
  unsigned long *sp;  

  SAVE_STATE();
  GetSP(sp);
  lwp_ptable[lwp_running].sp = sp;   /* GetSP(lwp_ptable[lwp_running].sp) */

  
  /* choose next process (round robin) */
  if(lwp_running+1 == lwp_procs) {
    lwp_running = 0;
  } else {
    lwp_running = lwp_running+1;
  }
  sp = lwp_ptable[lwp_running].sp;
  
  SetSP(sp);
  RESTORE_STATE();
  
  
}


/* Not workigng */
int new_lwp(lwpfun funct, void *ptr, size_t size)
{
  unsigned long *sp;

  lwp_ptable[lwp_procs].pid = lwp_procs+1;
  lwp_ptable[lwp_procs].stack = malloc(size*4);
  lwp_ptable[lwp_procs].stacksize = size;
  lwp_ptable[lwp_procs].sp = lwp_ptable[lwp_procs].stack + size;
  
  printf("size:\t%d\n", (int)sizeof(int));
  printf("sp:\t%d\n", (int)lwp_ptable[lwp_procs].sp);
  printf("stack:\t%d\n",(int)lwp_ptable[lwp_procs].stack);
  
  /* move stack pointer to high memory */
  /* decrement sp to get to next address */
  
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
  *(lwp_ptable[lwp_procs].sp) = sp;
    
  lwp_procs++;  
  
  //return lwp_ptable[lwp_procs-1].pid;
  return 1;

}

/*
 * Terminate current LWP
 */
void lwp_exit2() 
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
