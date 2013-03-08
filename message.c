
#include "message.h"
#include "message_pq.h"


void k_initMessages(MemInfo *memInfo, MessageInfo *messageInfo) {
    // TODO - allocate store as block...
    mpqInit(&(messageInfo->mpq), messageInfo->messageStore, 500);
}

void k_zeroEnvelope(Envelope *envelope) {
    envelope->next = NULL;
    envelope->sendTime = 0;
    envelope->srcPid = 0;
    envelope->dstPid = 0;
}

int8_t k_sendMessage(MemInfo *memInfo, ProcInfo *procInfo, ProcId pid, Envelope *envelope) {
    PCB *currentProc = NULL;
    PCB *receivingProc = NULL;

    k_zeroEnvelope(envelope);

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
    envelope->next = NULL;
    if (receivingProc->mqTail == NULL) {
        receivingProc->mqHead = envelope;
        receivingProc->mqTail = envelope;
    } else {
        receivingProc->mqTail->next = envelope;
        receivingProc->mqTail = envelope;
    }

    envelope->srcPid = currentProc->pid;
    envelope->dstPid = pid;

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
    PCB *currentProc = NULL;
    Envelope *message = NULL;

    // Check if message exists
    currentProc = procInfo->currentProcess;
    message = currentProc->mqHead;
    while (message == NULL) {
        // Block receiver
        k_releaseProcessor(procInfo, MESSAGE_RECEIVE);
        message = currentProc->mqHead;
    }

    // snip out of linked list
    currentProc->mqHead = message->next;
    if (currentProc->mqHead == NULL) {
        currentProc->mqTail = NULL;
    }

    // Clear this so user doesn't have next message pointer
    message->next = NULL;

    // Change ownership
    k_changeOwner(memInfo, (uint32_t)message, currentProc->pid);

    if (senderPid != NULL) {
        *senderPid = message->srcPid; // Set out param
    }
    return message;
}

int8_t k_delayedSend(MemInfo *memInfo, MessageInfo *messageInfo, ProcInfo *procInfo, uint8_t pid, Envelope *envelope, uint32_t delay) {
    k_zeroEnvelope(envelope);
    k_changeOwner(memInfo, (uint32_t)envelope, PROC_ID_KERNEL);

    envelope->sendTime = delay; // TODO(alex) - Set this to clock time + delay

    // TODO(sanjay): this seems to do no sanity checking of anything...

    envelope->srcPid = procInfo->currentProcess->pid;
    envelope->dstPid = pid;

    mpqAdd(&(messageInfo->mpq), envelope);
    return 0;
}

