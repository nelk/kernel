
#include "message.h"

int8_t k_sendMessage(MemInfo *memInfo, ProcInfo *procInfo, ProcId pid, Envelope *envelope) {
    PCB *currentProc;
    PCB *receivingProc;
    Envelope *nextMessage;

    // Check pid
    if (pid >= NUM_PROCS) {
        return 1;
    }
    // Set to new owner (and check if valid)
    if (k_changeOwner(memInfo, (uint32_t)envelope, PROC_ID_KERNEL) != 0) { // TODO (alex) - should we be using SUCCESS here? Maybe have global return codes like SUCCESS that's not just for memory?
        return 2;
    }

    currentProc = procInfo->currentProcess;
    receivingProc = &(procInfo->processes[pid]);

    // Add to message queue
    nextMessage = currentProc->endOfMessageQueue;
    envelope->header[NEXT_ENVELOPE] = (uint32_t)nextMessage;
    envelope->senderPid = currentProc->pid; // Force sender PID to be correct
    currentProc->endOfMessageQueue = envelope;
    if (currentProc->messageQueue == NULL) {
        currentProc->messageQueue = envelope;
    }

    // Unblock receiver
    if (receivingProc->state == BLOCKED_MESSAGE) {
        receivingProc->state = READY;
        pqAdd(&(procInfo->prq), receivingProc);
        // Preempt if unblocked process has higher priority - note that receiver is not guaranteed to run
        if (receivingProc->priority < currentProc->priority) {
            k_releaseProcessor(procInfo, MESSAGE_SENT);
        }
    }
    return 0;
}

Envelope *k_receiveMessage(MemInfo *memInfo, ProcInfo *procInfo, uint8_t *senderPid) {
    PCB *currentProc;
    Envelope *message;

    // Check if message exists
    currentProc = procInfo->currentProcess;
    while (currentProc->messageQueue == NULL) {
        // Block receiver
        k_releaseProcessor(procInfo, MESSAGE_RECEIVE);
    }
    message = currentProc->messageQueue;
    currentProc->messageQueue = (Envelope *)message->header[NEXT_ENVELOPE];
    if (currentProc->messageQueue == NULL) {
        currentProc->endOfMessageQueue = NULL;
    }
    message->header[NEXT_ENVELOPE] = 0; // Clear this so user doesn't have next message pointer
    k_changeOwner(memInfo, (uint32_t)message, currentProc->pid);

    if (senderPid != NULL) {
        *senderPid = message->senderPid; // Set out param
    }
    return message;
}

int8_t k_delayedSend(MemInfo *memInfo, ProcInfo *procInfo, uint8_t pid, Envelope *envelope, uint32_t delay) {
    // TODO
}
