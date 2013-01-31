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

ProcInfo procInfo;
extern MemInfo gMem;

void k_initProcesses(void) {
  PCB *process;
  ProcId i;

  pqInit(&procInfo.prq, procInfo.procQueue, NUM_PROCS);
  pqInit(&procInfo.memq, procInfo.memQueue, NUM_PROCS);

  for (i = 1; i < NUM_PROCS; ++i) {
    process = &procInfo.processes[i];
    process->pid = i;
    process->state = READY;
    // TODO(nelk): Assert that these memory blocks are contiguous
    k_acquireMemoryBlock(&gMem, i);
    process->stack = (uint32_t *)((uint32_t)k_acquireMemoryBlock(&gMem, i) + gMem.blockSizeBytes);
  }

  // Push process function address onto stack
  process = &procInfo.processes[0];
  --(process->stack);
  *(process->stack) = (uint32_t) nullProcess;
  process->priority = 4;
  process->state = RUNNING;
  pqAdd(&procInfo.prq, process);

  procInfo.currentProcess = NULL;
}

// TODO(sanjay): make this API cleaner.
uint32_t k_releaseProcessor(ReleaseReason reason) {
  PCB *nextProc = NULL;

  // we need to set these three variables depending on the release reason
  ProcState targetState = READY;
  // we pull the next process to execute from the source queue
  PQ *srcQueue = NULL;
  // we push the currently executing process onto this queue
  PQ *dstQueue = NULL;


  switch (reason) {
  case MEMORY_FREED:
    srcQueue = &(procInfo->memq);
    dstQueue = &(procInfo->prq);
    targetState = READY;
    break;
  case OOM:
    srcQueue = &(procInfo->prq);
    dstQueue = &(procInfo->memq);
    targetState = BLOCKED;
    break;
  case YIELD:
    srcQueue = &(procInfo->prq);
    dstQueue = &(procInfo->prq);
    targetState = READY;
    break;
  default:
    break;
  }

  nextProc = pqTop(queue);
  pqRemove(queue, 0);

  if (procInfo.currentProcess != NULL) {
    // Save old process info
    procInfo.currentProcess->stack = (uint32_t *) __get_MSP();
    procInfo.currentProcess->state = targetState;
    if (dstQueue != NULL) {
      pqAdd(dstQueue, procInfo.currentProcess);
    }
  }

  nextProc->state = RUNNING;
  procInfo.currentProcess = nextProc;
  __set_MSP((uint32_t) nextProc->stack);
  __rte();

  return 0;
}

uint32_t k_setProcessPriority(ProcId pid, uint8_t priority) {
  if (procInfo.currentProcess == NULL ||
      procInfo.currentProcess->pid != pid) {
    return 1;
  }

  // TODO(sanjay): constantify
  if (priority >= 4) {
    return 2;
  }

  procInfo.currentProcess->pid = priority;
  return 0;
}

uint32_t k_getProcessPriority(ProcId pid) {
  if (pid >= NUM_PROCS) {
    return ~0;
  }

  return procInfo.processes[pid].priority;
}
