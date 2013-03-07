#ifndef BRIDGE_H
#define BRIDGE_H

#include "message.h"
#include "types.h"

uint32_t bridge_releaseProcessor(void);

uint32_t bridge_setProcessPriority(uint8_t pid, uint8_t priority);
int16_t bridge_getProcessPriority(uint8_t pid);

void *bridge_acquireMemoryBlock(void);
void *bridge_tryAcquireMemoryBlock(void);
int8_t bridge_releaseMemoryBlock(void *blk);

int8_t bridge_sendMessage(uint8_t pid, void *envelope);
Envelope *bridge_receiveMessage(uint8_t *senderPid);
int8_t bridge_delayedSend(uint8_t pid, void *envelope, uint32_t delay);

uint32_t bridge_getTime(void);

#endif
