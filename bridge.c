#include <stdint.h>

#include "bridge.h"
#include "mem.h"
#include "proc.h"

extern MemInfo gMem;
extern ProcInfo procInfo;

uint32_t bridge_releaseProcessor(void) {
  return k_releaseProcessor(&procInfo, YIELD);
}

uint32_t bridge_setProcessPriority(uint8_t pid, uint8_t priority) {
  return k_setProcessPriority(&procInfo, (ProcId)pid, priority);
}

int16_t bridge_getProcessPriority(uint8_t pid) {
  return k_getProcessPriority(&procInfo, pid);
}


void *bridge_acquireMemoryBlock(void) {
  return k_acquireMemoryBlock(&gMem, &procInfo, procInfo.currentProcess->pid);
}

int8_t bridge_releaseMemoryBlock(void *blk) {
  return k_releaseMemoryBlock(&gMem, &procInfo, blk, procInfo.currentProcess->pid);
}

uint8_t bridge_isOutOfMemory(void) {
    return k_isOutOfMemory(&gMem);
}

