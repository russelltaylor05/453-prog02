/* Russell Taylor
 * Alex Spotnitz
 *
 * CPE 453
 * Dr. Nico
 *
 * Assignment 2: Lightweight Processes
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/wait.h>

#include "lwp.h"

void print_table();
int roundrobin();

/* Global variable to define max number of active LWPs */
lwp_context lwp_ptable[LWP_PROC_LIMIT];

/* Main/original stack pointer to return to after lwp_stop() */
unsigned long *initial; 

/* scheduler function */
schedfun scheduler;

/*
 * Starts the LWP system.
 *
 * Saves the original context and stack pointer (for
 * lwp_stop() to use later), picks a LWP and starts
 * it running. If there are no LWPs, returns immediately.
 */
void lwp_start()
{
  /* If there are no LWPs, return immediately. */
  if(lwp_procs == 0){
      return;
  }
  SAVE_STATE();
  GetSP(initial);
  SetSP(lwp_ptable[0].sp);
  lwp_running = 0;
  RESTORE_STATE();
}

/* Stops the LWP system.
 *
 * Restores the original stack pointer and returns to that
 * context (wherever lwp_start() was called from).
 */
void lwp_stop()
{  
  SAVE_STATE();
  GetSP(lwp_ptable[lwp_running].sp);
  SetSP(initial);
  RESTORE_STATE();
}
    
/* Yields control to another LWP (chosen by the scheduler).
 *
 * Saves the current LWP's context, picks the next one, restores
 * that thread's context, and returns.
 */
void lwp_yield()
{

  SAVE_STATE();
  GetSP(lwp_ptable[lwp_running].sp);
    
  if(scheduler != NULL){
     lwp_running = scheduler();
  } else {
     lwp_running = (lwp_running + 1) % lwp_procs;
  } 

  SetSP(lwp_ptable[lwp_running].sp);
  RESTORE_STATE();  
}


int new_lwp(lwpfun funct, void *ptr, size_t size)
{
  unsigned long *sp;
  int i;
  i = 0;

  /* If more than LWP_PROC_LIMIT threads already exist,
   * then do not create a new thread/LWP. */
  if(lwp_procs > LWP_PROC_LIMIT){
      return -1;
  }

  lwp_ptable[lwp_procs].pid = lwp_procs + 1;
  /* size is number of words. 1 words = 4 bytes: */
  lwp_ptable[lwp_procs].stack = (unsigned long*)malloc(size*4);
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

  return (int)lwp_ptable[lwp_procs-1].pid;

}

/*
 * Terminates the current LWP.
 *
 * Frees its resources, and moves all the others up in the process
 * table.
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
    return;
  }
  
  /* Move processes in table down */
  for (i = lwp_running; i < lwp_procs; i++) {
    lwp_ptable[i] = lwp_ptable[i+1];
  }

  /* Choose new process */
  sp = lwp_ptable[lwp_running].sp;
  SetSP(sp);
  RESTORE_STATE(); /* make last statement */

}

/*
 * Causes the LWP package to use the function scheduler to choose
 * the next process to run.
 */

void lwp_set_scheduler(schedfun sched) 
{
  scheduler = sched;
}


/* 
 * Returns the PID of the calling LWP.
 */
unsigned long lwp_getpid()
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
