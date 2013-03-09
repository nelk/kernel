#include <stdint.h>

#include "bridge.h"
#include "mem.h"
#include "message.h"
#include "proc.h"
#include "pq.h"
#include "timer.h"

extern ClockInfo gClockInfo;
extern MemInfo gMemInfo;
extern MessageInfo gMessageInfo;
extern ProcInfo gProcInfo;

uint32_t bridge_releaseProcessor(void) {
    return k_releaseProcessor(&gProcInfo, &gMemInfo, &gMessageInfo, &gClockInfo, YIELD);
}

uint32_t bridge_setProcessPriority(uint8_t pid, uint8_t priority) {
    return k_setProcessPriority(&gProcInfo, &gMemInfo, &gMessageInfo, &gClockInfo, pid, priority);
}

int16_t bridge_getProcessPriority(uint8_t pid) {
    return k_getProcessPriority(&gProcInfo, pid);
}


void *bridge_tryAcquireMemoryBlock(void) {
    return (void *)k_acquireMemoryBlock(&gMemInfo, gProcInfo.currentProcess->pid);
}

void *bridge_acquireMemoryBlock(void) {
    uint32_t mem = 0;

    mem = k_acquireMemoryBlock(&gMemInfo, gProcInfo.currentProcess->pid);
    while (mem == NULL) {
        k_releaseProcessor(&gProcInfo, &gMemInfo, &gMessageInfo, &gClockInfo, OOM);
        mem = k_acquireMemoryBlock(&gMemInfo, gProcInfo.currentProcess->pid);
    }

    return (void*)mem;
}

int8_t bridge_releaseMemoryBlock(void *blk) {
    int8_t status = SUCCESS;
    PCB *firstBlocked = NULL;

    status = k_releaseMemoryBlock(&gMemInfo, (uint32_t)blk, gProcInfo.currentProcess->pid);
    if (status != SUCCESS) {
        return status;
    }

    if (gProcInfo.memq.size == 0) {
        return SUCCESS;
    }

    firstBlocked = pqTop(&(gProcInfo.memq));
    if (firstBlocked->priority >= gProcInfo.currentProcess->priority) {
        return SUCCESS;
    }

    k_releaseProcessor(&gProcInfo, &gMemInfo, &gMessageInfo, &gClockInfo, MEMORY_FREED);
    return SUCCESS;
}


int8_t bridge_sendMessage(uint8_t pid, Envelope *envelope) {
    int8_t releaseProcessor = k_sendMessage(&gMemInfo, &gProcInfo, envelope, gProcInfo.currentProcess->pid, pid);
    if (releaseProcessor == -1) {   // TODO: Replace with enums.
        k_releaseProcessor(&gProcInfo, &gMemInfo, &gMessageInfo, &gClockInfo, MESSAGE_SENT);
    }
    return releaseProcessor;
}

Envelope *bridge_receiveMessage(uint8_t *senderPid) {
    Envelope *envelope = k_receiveMessage(&gMessageInfo, &gMemInfo, &gProcInfo, &gClockInfo);
    if (senderPid != NULL && envelope != NULL) {
        *senderPid = envelope->srcPid; // Set out param
    }
    return envelope;
}

int8_t bridge_delayedSend(uint8_t pid, Envelope *envelope, uint32_t delay) {
    return k_delayedSend(&gMessageInfo, &gMemInfo, pid, envelope, delay);
}


uint32_t bridge_getTime(void) {
    return k_getTime(&gClockInfo);
}

ProcId bridge_getPid(void) {
    return k_getPid(&gProcInfo);
}
