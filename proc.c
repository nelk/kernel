
#include <stdint.h>
#include <stddef.h>
//#include "LPC17xx.h"
#include "proc.h"
#include "mem.h"
#include "prq.h"

void __rte(void);
void  __set_MSP(uint32_t);
uint32_t __get_MSP(void);

PRQ prq; // Process ready queue
PCB processes[NUM_PROCS]; // Actual process blocks
PCB *procQueue[NUM_PROCS];
PCB *currentProcess = NULL;

void nullProcess() {
  while (1) {
    releaseProcessor();
  }
}

void funProcess() {
  int i;
  for (i = 0; i < 100; ++i) {
		//uart0_put_string("Hi\n\r");
    releaseProcessor();
  }
}

void k_initProcesses() {
  PCB *process;
  uint32_t i;

  prqInit(prq, procQueue, NUM_PROCS);

  for (i = 0; i < NUM_PROCS; ++i) {
    process = &processes[i];
    process->pid = i;
    process->state = READY;
    //TODO - assert that these memory blocks are contigious
    k_acquireMemoryBlock(i);
    process->stack = k_acquireMemoryBlock(i) + gMem.blockSizeBytes;
  }

  // Push process function address onto stack
  process = &processes[0];
  process->stack--;
  *(process->stack) = (uint32_t) nullProcess;
  process->priority = 6;
  process->state = RUNNING;
  prqAdd(prq, process);

  currentProcess = NULL;
}

int releaseProcessor() {
  if (currentProcess != NULL) {
    // Save old process info
    currentProcess->state = READY;
    currentProcess->stack = (uint32_t *) __get_MSP(); /* save the old process's sp */
    prqAdd(prq, currentProcess);
  }

  PCB *nextProc = prqTop(prq);
  nextProc->state = RUNNING;
  prqRemove(prq, 0);
  currentProcess = nextProc;
  __set_MSP((uint32_t) nextProc->stack);
  __rte();

  return 0; // Not reachable
}


