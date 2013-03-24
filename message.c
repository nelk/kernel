#include "kernel_types.h"
#include "message.h"
#include "message_pq.h"
#include "timer.h"

void k_initMessages(MessageInfo *messageInfo, MemInfo *memInfo) {
    // TODO - allocate store as block...
    mpqInit(&(messageInfo->mpq), messageInfo->messageStore, 500);
}

void k_zeroEnvelope(Envelope *envelope) {
    envelope->next = NULL;
    envelope->sendTime = 0;
    envelope->srcPid = 0;
    envelope->dstPid = 0;
}

int8_t k_sendMessage(MemInfo *memInfo, ProcInfo *procInfo, Envelope *envelope, ProcId srcPid, ProcId dstPid) {
    PCB *dstPCB = NULL;
		PCB *srcPCB = NULL;
		uint8_t status = 0;

    k_zeroEnvelope(envelope);

		dstPCB = k_getPCB(procInfo, dstPid);
		if (dstPCB == NULL) {
				return EINVAL;
		}
		
		srcPCB = k_getPCB(procInfo, srcPid);
		if (srcPCB == NULL) {
				return EINVAL;
		}
		
		status = k_changeOwner(memInfo, (uint32_t)envelope, srcPid, PROC_ID_KERNEL);
	
    // Set to new owner (and check if valid)
    if (status != SUCCESS) { // TODO (alex) - should we be using SUCCESS here? Maybe have global return codes like SUCCESS that's not just for memory?
        return status;
    }

    // Add to message queue
    envelope->next = NULL;
    if (dstPCB->mqTail == NULL) {
        dstPCB->mqHead = envelope;
        dstPCB->mqTail = envelope;
    } else {
        dstPCB->mqTail->next = envelope;
        dstPCB->mqTail = envelope;
    }

    envelope->srcPid = srcPid;
    envelope->dstPid = dstPid;

    // Unblock receiver
    if (dstPCB->state == BLOCKED_MESSAGE) {
        dstPCB->state = READY;
        pqAdd(&(procInfo->prq), dstPCB);
        // Preempt if unblocked process has higher priority - note that receiver is not guaranteed to run
        if (dstPCB->priority < procInfo->currentProcess->priority) {
            return -1;
        }
    }
    return 0;
}

Envelope *k_receiveMessage(MessageInfo *messageInfo, MemInfo *memInfo, ProcInfo *procInfo, ClockInfo *clockInfo) {
    PCB *currentProc = NULL;
    Envelope *message = NULL;

    // Check if message exists
    currentProc = procInfo->currentProcess;
    message = currentProc->mqHead;
    while (message == NULL) {
        // Block receiver
        k_releaseProcessor(procInfo, memInfo, messageInfo, clockInfo, MESSAGE_RECEIVE);
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
    k_changeOwner(memInfo, (uint32_t)message, PROC_ID_KERNEL, currentProc->pid);

    return message;
}

int8_t k_sendDelayedMessage(MessageInfo *messageInfo, ClockInfo *clockInfo, MemInfo *memInfo, ProcInfo *procInfo, Envelope *envelope, ProcId srcPid, ProcId dstPid, uint32_t delay) {
    k_zeroEnvelope(envelope);
    // Check pid - the src is from the bridge, but why not...
    if (dstPid >= NUM_PROCS || srcPid >= NUM_PROCS) {
        return 1;
    }

    envelope->sendTime = k_getTime(clockInfo) + delay;

    // TODO(sanjay): this seems to do no sanity checking of anything...

    envelope->srcPid = srcPid;
    envelope->dstPid = dstPid;

    mpqAdd(&(messageInfo->mpq), envelope);
    return 0;
}

void k_processDelayedMessages(MessageInfo *messageInfo, ProcInfo *procInfo, MemInfo *memInfo, ClockInfo *clockInfo) {
    MessagePQ *messageQueue = &(messageInfo->mpq);
    Envelope *message = NULL;
    uint32_t currentTime = k_getTime(clockInfo);

    if (messageQueue == NULL || messageQueue->size <= 0) {
        return;
    }

    while (1) {
        message = mpqTop(messageQueue);

        if (message == NULL || message->sendTime > currentTime) {
            return;
        }

        mpqRemove(messageQueue, 0);
        k_sendMessage(memInfo, procInfo, message, message->srcPid, message->dstPid);
    }
}
