#include <stdint.h>

#include "bridge.h"
#include "mem.h"
#include "proc.h"

extern MemInfo gMem;
extern ProcInfo procInfo;

uint32_t bridge_releaseProcessor(void) {
  return k_releaseProcessor(YIELD);
}

void *bridge_acquireMemoryBlock(void) {
  return k_acquireMemoryBlock(&gMem, &procInfo, procInfo.currentProcess->pid);
}

uint32_t bridge_releaseMemoryBlock(void *blk) {
  return k_releaseMemoryBlock(&gMem, &procInfo, blk, procInfo.currentProcess->pid);
}

