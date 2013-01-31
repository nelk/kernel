#ifndef BRIDGE_H
#define BRIDGE_H

uint32_t bridge_releaseProcessor(void);
void *bridge_acquireMemoryBlock(void);
uint32_t bridge_releaseMemoryBlock(void *blk);

#endif

