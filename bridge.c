#include <stdint.h>

#include "bridge.h"
#include "mem.h"
#include "proc.h"
#include "message.h"

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


void *bridge_tryAcquireMemoryBlock(void) {
    return k_acquireMemoryBlock(&gMem, procInfo.currentProcess->pid);
}

void *bridge_acquireMemoryBlock(void) {
    void *mem = NULL;

    mem = k_acquireMemoryBlock(&gMem, procInfo.currentProcess->pid);
    while (mem == NULL) {
        k_releaseProcessor(&procInfo, OOM);
        mem = k_acquireMemoryBlock(&gMem, procInfo.currentProcess->pid);
    }

    return mem;
}

int8_t bridge_releaseMemoryBlock(void *blk) {
    int8_t status = SUCCESS;
    PCB *firstBlocked = NULL;

    status = k_releaseMemoryBlock(&gMem, blk, procInfo.currentProcess->pid);
    if (status != SUCCESS) {
        return status;
    }

    if (procInfo.memq.size == 0) {
        return SUCCESS;
    }

    firstBlocked = pqTop(&(procInfo.memq));
    if (firstBlocked->priority >= procInfo.currentProcess->priority) {
        return SUCCESS;
    }

    k_releaseProcessor(&procInfo, MEMORY_FREED);
    return SUCCESS;
}


int8_t bridge_sendMessage(uint8_t pid, Envelope *envelope) {
    k_sendMessage(&gMem, &procInfo, pid, envelope);
}

int8_t bridge_receiveMessage(uint8_t *senderPid) {
    k_receiveMessage(&gMem, &procInfo, senderPid);
}

int8_t bridge_delayedSend(uint8_t pid, Envelop *envelope, uint32_t delay) {
    k_delayedSend(&gMem, &procInfo, pid, envelope, delay);
}
