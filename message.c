
#include "message.h"

int8_t k_sendMessage(MemInfo *memInfo, ProcInfo *procInfo, ProcId pid, Envelope *envelope) {
    PCB *currentProc;
    PCB *receivingProc;
    Envelope *nextMessage;

    // Check pid
    if (pid >= NUM_PROCS) {
        return 1;
    }
    // Check memory block
    if (k_validateMemoryBlock(memInfo, envelope, pid) != 0) { // TODO (alex) - should we be using SUCCESS here? Maybe have global return codes like SUCCESS that's not just for memory?
        return 2;
    }

    // Set to new owner
    k_setOwner(memInfo, (uint32_t)envelope, pid);

    currentProc = procInfo->currentProcess;
    receivingProc = &(procInfo->processes[pid]);

    // Add to message queue
    nextMessage = currentProc->messageQueue;
    envelope->header[NEXT_ENVELOPE] = (uint32_t)nextMessage;
    currentProc->messageQueue = envelope;

    // Unblock receiver
    if (receivingProc->state == BLOCKED_MESSAGE) {
        receivingProc->state = READY;
        pqAdd(&(procInfo->prq), receivingProc);
        // Preempt if unblocked process has higher priority - note that receiver is not guaranteed to run
        if (receivingProc->priority < currentProc->priority) {
            k_releaseProcessor(procInfo, MESSAGE_RECEIVED);
        }
    }
    return 0;
}

int8_t k_receiveMessage(MemInfo *memInfo, ProcInfo *procInfo, uint8_t *senderPid) {
    // TODO
}

int8_t k_delayedSend(MemInfo *memInfo, ProcInfo *procInfo, uint8_t pid, Envelope *envelope, uint32_t delay) {
    // TODO
}
