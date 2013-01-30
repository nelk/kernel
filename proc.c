
#include <stdint.h>
#include <stddef.h>

#include <LPC17xx.h>
#include "mem.h"
#include "proc.h"
#include "prq.h"
#include "user.h"

void __rte(void);
void  __set_MSP(uint32_t);
uint32_t __get_MSP(void);

typedef struct ProcInfo ProcInfo;
struct ProcInfo {
  PRQ prq; // Process ready queue
  PCB processes[NUM_PROCS]; // Actual process blocks
  PCB *procQueue[NUM_PROCS];
  PCB *currentProcess;
};

ProcInfo procInfo;

void k_initProcesses(void) {
  PCB *process;
  ProcId i;

  prqInit(&procInfo.prq, procInfo.procQueue, NUM_PROCS);

  for (i = 0; i < NUM_PROCS; ++i) {
    process = &procInfo.processes[i];
    process->pid = i;
    process->state = READY;
    // TODO(nelk): Assert that these memory blocks are contiguous
    k_acquireMemoryBlock(i);
    process->stack = (uint32_t *)((uint32_t)k_acquireMemoryBlock(i) + gMem.blockSizeBytes);
  }

  // Push process function address onto stack
  process = &procInfo.processes[0];
  process->stack--;
  *(process->stack) = (uint32_t) nullProcess;
  process->priority = 6;
  process->state = RUNNING;
  prqAdd(&procInfo.prq, process);

  procInfo.currentProcess = NULL;
}

uint32_t k_releaseProcessor(void) {
  PCB *nextProc = prqTop(&procInfo.prq);

  if (procInfo.currentProcess != NULL) {
    // Save old process info
    procInfo.currentProcess->state = READY;
    procInfo.currentProcess->stack = (uint32_t *) __get_MSP(); // save the old process's sp
    prqAdd(&procInfo.prq, procInfo.currentProcess);
  }

  nextProc->state = RUNNING;
  prqRemove(&procInfo.prq, 0);
  procInfo.currentProcess = nextProc;
  __set_MSP((uint32_t) nextProc->stack);
  __rte();

  return 0; // Not reachable
}


