#include <stdint.h>

#include "bridge.h"
#include "mem.h"
#include "proc.h"
#include "timer.h"

extern ClockInfo gClockInfo;
extern MemInfo gMem;
extern MessageInfo gMessageInfo;
extern ProcInfo gProcInfo;

uint32_t bridge_releaseProcessor(void) {
    return k_releaseProcessor(&gProcInfo, YIELD);
}

uint32_t bridge_setProcessPriority(uint8_t pid, uint8_t priority) {
    return k_setProcessPriority(&gProcInfo, pid, priority);
}

int16_t bridge_getProcessPriority(uint8_t pid) {
    return k_getProcessPriority(&gProcInfo, pid);
}


void *bridge_tryAcquireMemoryBlock(void) {
    return k_acquireMemoryBlock(&gMem, gProcInfo.currentProcess->pid);
}

void *bridge_acquireMemoryBlock(void) {
    void *mem = NULL;

    mem = k_acquireMemoryBlock(&gMem, gProcInfo.currentProcess->pid);
    while (mem == NULL) {
        k_releaseProcessor(&gProcInfo, OOM);
        mem = k_acquireMemoryBlock(&gMem, gProcInfo.currentProcess->pid);
    }

    return mem;
}

int8_t bridge_releaseMemoryBlock(void *blk) {
    int8_t status = SUCCESS;
    PCB *firstBlocked = NULL;

    status = k_releaseMemoryBlock(&gMem, (uint32_t)blk, gProcInfo.currentProcess->pid);
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

    k_releaseProcessor(&gProcInfo, MEMORY_FREED);
    return SUCCESS;
}


int8_t bridge_sendMessage(uint8_t pid, Envelope *envelope) {
    return k_sendMessage(&gMem, &gProcInfo, pid, envelope);
}

Envelope *bridge_receiveMessage(uint8_t *senderPid) {
    return k_receiveMessage(&gMem, &gProcInfo, senderPid);
}

int8_t bridge_delayedSend(uint8_t pid, Envelope *envelope, uint32_t delay) {
    return k_delayedSend(&gMem, &gMessageInfo, &gProcInfo, pid, envelope, delay);
}

uint32_t bridge_getTime(void) {
    return k_getTime(gClockInfo);
}
