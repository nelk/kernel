#ifndef BRIDGE_H
#define BRIDGE_H

uint32_t bridge_releaseProcessor(void);
uint32_t bridge_setProcessPriority(uint8_t pid, uint8_t priority);
uint8_t bridge_getProcessPriority(uint8_t pid);

void *bridge_acquireMemoryBlock(void);
uint32_t bridge_releaseMemoryBlock(void *blk);

#endif

