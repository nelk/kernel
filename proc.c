#include <stdint.h>
#include <stddef.h>

#include <LPC17xx.h>
#include "mem.h"
#include "proc.h"
#include "pq.h"
#include "user.h"

extern void __rte(void);
extern void  __set_MSP(uint32_t);
extern uint32_t __get_MSP(void);

extern MemInfo gMem;

void k_initProcesses(ProcInfo *procInfo) {
  PCB *process;
  ProcId i;
	int j;
	uint32_t *stack = NULL;

  pqInit(&(procInfo->prq), procInfo->procQueue, NUM_PROCS);
  pqInit(&(procInfo->memq), procInfo->memQueue, NUM_PROCS);

  for (i = 0; i < NUM_PROCS; ++i) {
    process = &(procInfo->processes[i]);
    process->pid = i;
    process->state = NEW;
    // TODO(nelk): Assert that these memory blocks are contiguous
    k_acquireMemoryBlock(&gMem, procInfo, i);     
		
		stack = (uint32_t *)(
			(uint32_t)k_acquireMemoryBlock(&gMem, procInfo, i) + gMem.blockSizeBytes
    );
	
		if (!(((uint32_t)stack) & 0x04)) {
				--stack; 
		}
		
		*(--stack) = 0x01000000; // <- does this work? (this is called XPsr)
		process->startLoc = --stack;
		
		for (j = 0; j < 6; j++) {
			*(--stack) = 0x0;
		}
		
		if (!(((uint32_t)stack) & 0x04)) {
				//--stack; 
		}
		process->stack = stack;
  }

  // Null Process
  process = &(procInfo->processes[0]); // Push process function address onto stack	
  *(process->startLoc) = ((uint32_t) nullProcess);
  process->priority = 4;
  procInfo->nullProcess = process;
	
	// Fun Process
  process = &(procInfo->processes[1]); // Push process function address onto stack
  *(process->startLoc) = ((uint32_t) funProcess);
  process->priority = 3;
	pqAdd(&(procInfo->prq), process);
	
	// Schizo Process
  process = &(procInfo->processes[2]); // Push process function address onto stack
  *(process->startLoc) = ((uint32_t) schizophrenicProcess);
  process->priority = 2;
	pqAdd(&(procInfo->prq), process);

  procInfo->currentProcess = NULL;
}

uint32_t k_releaseProcessor(ProcInfo *k_procInfo, ReleaseReason reason) {
  PCB *nextProc = NULL;
	ProcState oldState = NEW;

  // We need to set these three variables depending on the release reason
  ProcState targetState = READY;
  // We pull the next process to execute from the source queue
  PQ *srcQueue = NULL;
  // We push the currently executing process onto this queue
  PQ *dstQueue = NULL;


  switch (reason) {
  case MEMORY_FREED:
    srcQueue = &(k_procInfo->memq);
    dstQueue = &(k_procInfo->prq);
    targetState = READY;
    break;
  case OOM:
    srcQueue = &(k_procInfo->prq);
    dstQueue = &(k_procInfo->memq);
    targetState = BLOCKED;
    break;
  case YIELD:
    srcQueue = &(k_procInfo->prq);
    dstQueue = &(k_procInfo->prq);
    targetState = READY;

    // If it was the null process that yielded, we don't add it to the ready queue.
    if (k_procInfo->currentProcess == k_procInfo->nullProcess) {
      dstQueue = NULL;
    }
    break;
  default:
    break;
  }

  if (srcQueue != NULL && srcQueue->size > 0) {
    nextProc = pqTop(srcQueue);
    pqRemove(srcQueue, 0);
  } else {
    nextProc = k_procInfo->nullProcess;
  }

  if (k_procInfo->currentProcess != NULL) {
    // Save old process info
    k_procInfo->currentProcess->stack = (uint32_t *) __get_MSP();
    k_procInfo->currentProcess->state = targetState;
    if (dstQueue != NULL) {
      pqAdd(dstQueue, k_procInfo->currentProcess);
    }
  }

	oldState = nextProc->state;
  nextProc->state = RUNNING;
  k_procInfo->currentProcess = nextProc;
  __set_MSP((uint32_t) nextProc->stack);
	if (oldState == NEW) {
		__rte();
	}

  return 0;
}

uint32_t k_setProcessPriority(ProcInfo *procInfo, ProcId pid, uint8_t priority) {
  if (procInfo->currentProcess == NULL ||
      procInfo->currentProcess->pid != pid) {
    return 1;
  }

  // TODO(sanjay): constantify
  if (priority >= 4) {
    return 2;
  }

  procInfo->currentProcess->pid = priority;
  return 0;
}

uint32_t k_getProcessPriority(ProcInfo *procInfo, ProcId pid) {
  if (pid >= NUM_PROCS) {
    return ~0;
  }

  return procInfo->processes[pid].priority;
}

