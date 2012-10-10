#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/wait.h>

#include "lwp.h"




int main(){
  int i;
  int arr[150];
  
  int *ptr = malloc(4);
  
  ptr = ptr + 13;
  
  *ptr = 10;
  ptr++;
  free(ptr);
  
  
  return 1;

}