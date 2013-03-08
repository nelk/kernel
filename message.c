
#include "message.h"
#include "message_pq.h"
#include "timer.h"

void k_initMessages(MemInfo *memInfo, MessageInfo *messageInfo) {
    // TODO - allocate store as block...
    mpqInit(&(messageInfo->mpq), messageInfo->messageStore, 500);
}

int8_t k_sendMessage(MemInfo *memInfo, ProcInfo *procInfo, Envelope *envelope, ProcId receiverPid, ProcId senderPid) {
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

    receivingProc = &(procInfo->processes[pid]);

    // Add to message queue
    nextMessage = receivingProc->endOfMessageQueue;
    envelope->header[NEXT_ENVELOPE] = (uint32_t)nextMessage;
    envelope->senderPid = senderPid; // Force sender PID to be correct
    envelope->destPid = receiverPid;
    receivingProc->endOfMessageQueue = envelope;
    if (receivingProc->messageQueue == NULL) {
        receivingProc->messageQueue = envelope;
    }

    // Unblock receiver
    if (receivingProc->state == BLOCKED_MESSAGE) {
        receivingProc->state = READY;
        pqAdd(&(procInfo->prq), receivingProc);
        // Preempt if unblocked process has higher priority - note that receiver is not guaranteed to run
        if (receivingProc->priority < procInfo->currentProcess->priority) {
            return -1;
        }
    }
    return 0;
}

Envelope *k_receiveMessage(MemInfo *memInfo, ProcInfo *procInfo, MessageInfo *messageInfo, ClockInfo *clockInfo) {
    PCB *currentProc;
    Envelope *message;

    // Check if message exists
    currentProc = procInfo->currentProcess;
    while (currentProc->messageQueue == NULL) {
        // Block receiver
        k_releaseProcessor(memInfo, procInfo, messageInfo, clockInfo, MESSAGE_RECEIVE);
    }
    message = currentProc->messageQueue;
    currentProc->messageQueue = (Envelope *)message->header[NEXT_ENVELOPE];
    if (currentProc->messageQueue == NULL) {
        currentProc->endOfMessageQueue = NULL;
    }
    message->header[NEXT_ENVELOPE] = 0; // Clear this so user doesn't have next message pointer
    k_changeOwner(memInfo, (uint32_t)message, currentProc->pid);

    return message;
}

int8_t k_delayedSend(MemInfo *memInfo, MessageInfo *messageInfo, ProcInfo *procInfo, uint8_t pid, Envelope *envelope, uint32_t delay) {
    k_changeOwner(memInfo, (uint32_t)envelope, PROC_ID_KERNEL);
    envelope->header[SEND_TIME] = k_getTime() + delay;
    envelope->senderPid = procInfo->currentProcess->pid;
    envelope->destPid = pid;

    mpqAdd(&(messageInfo->mpq), envelope);
    return 0;
}

