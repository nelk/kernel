#include <stddef.h>
#include <stdint.h>

#include <LPC17xx.h>

#include "crt.h"
#include "helpers.h"
#include "kernel_types.h"
#include "mem.h"
#include "message.h"
#include "proc.h"
#include "pq.h"
#include "timer.h"
#include "uart.h"
#include "user.h"

extern void __rte(void);
extern void  __set_MSP(uint32_t);
extern uint32_t __get_MSP(void);

ssize_t *rqStoreIndexFunc(PCB *pcb) {
    return &(pcb->rqIndex);
}

ssize_t *memqStoreIndexFunc(PCB *pcb) {
    return &(pcb->memqIndex);
}

void k_initProcesses(ProcInfo *procInfo, MemInfo *memInfo) {
    ProcId i = 0;
    int j = 0;
    PCB *process = NULL;
    uint32_t *stack = NULL;
    uint8_t pidOffset = 0;

    memset((uint8_t *)procInfo, sizeof(ProcInfo), 0);

    pqInit(&(procInfo->prq), procInfo->procQueue, NUM_PROCS, &rqStoreIndexFunc);
    pqInit(&(procInfo->memq), procInfo->memQueue, NUM_PROCS, &memqStoreIndexFunc);

    for (i = 0; i < NUM_PROCS; ++i) {
        uint32_t tempStack = 0;
        process = &(procInfo->processes[i]);
        process->pid = i;
        process->state = UNUSED;
        process->mqHead = NULL;
        process->mqTail = NULL;
        process->debugEnv = NULL;

        // TODO(nelk): Assert that these memory blocks are contiguous
        // Stack grows backwards, not forwards. We allocate three memory blocks.
        tempStack = k_acquireMemoryBlock(memInfo, PROC_ID_KERNEL);
        tempStack = k_acquireMemoryBlock(memInfo, PROC_ID_KERNEL);
				tempStack = k_acquireMemoryBlock(memInfo, PROC_ID_KERNEL);

        stack = (uint32_t *)(tempStack + memInfo->blockSizeBytes);

        if (!(((uint32_t)stack) & 0x04)) {
            --stack;
        }

        *(--stack) = 0x01000000; // <- does this work? (this is called XPsr)
        process->startLoc = --stack;

        // NOTE(sanjay): We use the "zero function" as a marker for an unused
        // process. This address can never be a valid start location for a
        // process, as it is somewhere inside the vector table...
        *(process->startLoc) = 0;

        for (j = 0; j < 6; j++) {
            *(--stack) = 0x0;
        }

        process->stack = stack;
    }

    // Null Process
    process = &(procInfo->processes[NULL_PID]); // Push process function address onto stack
    *(process->startLoc) = ((uint32_t) nullProcess);
    process->priority = (2 << KERN_PRIORITY_SHIFT) | MAX_PRIORITY;
    procInfo->nullProcess = process;
    process->debugEnv = (Envelope *)k_acquireMemoryBlock(memInfo, CRT_PID);
		process->state = NEW;

    // Clock Process
    process = &(procInfo->processes[CLOCK_PID]); // Push process function address onto stack
    *(process->startLoc) = ((uint32_t) clockProcess);
    process->priority = (0 << KERN_PRIORITY_SHIFT) | 2;
    pqAdd(&(procInfo->prq), process);
    process->debugEnv = (Envelope *)k_acquireMemoryBlock(memInfo, CRT_PID);
		process->state = NEW;

    // CRT Process
    process = &(procInfo->processes[CRT_PID]); // Push process function address onto stack
    *(process->startLoc) = ((uint32_t) crt_proc);
    process->priority = (0 << KERN_PRIORITY_SHIFT) | 0;
    pqAdd(&(procInfo->prq), process);
    process->debugEnv = (Envelope *)k_acquireMemoryBlock(memInfo, CRT_PID);
		process->state = NEW;

    // Keyboard Process
    process = &(procInfo->processes[KEYBOARD_PID]); // Push process function address onto stack
    *(process->startLoc) = ((uint32_t) uart_keyboard_proc);
    process->priority = (0 << KERN_PRIORITY_SHIFT) | 1;
    pqAdd(&(procInfo->prq), process);
    process->debugEnv = (Envelope *)k_acquireMemoryBlock(memInfo, CRT_PID);
		process->state = NEW;

    // Fun Process
    process = &(procInfo->processes[FIRST_USER_PID + pidOffset++]); // Push process function address onto stack
    *(process->startLoc) = ((uint32_t) funProcess);
    process->priority = (1 << KERN_PRIORITY_SHIFT) | 3;
    pqAdd(&(procInfo->prq), process);
    process->debugEnv = (Envelope *)k_acquireMemoryBlock(memInfo, CRT_PID);
		process->state = NEW;

    // Schizo Process
    process = &(procInfo->processes[FIRST_USER_PID + pidOffset++]); // Push process function address onto stack
    *(process->startLoc) = ((uint32_t) schizophrenicProcess);
    process->priority = (1 << KERN_PRIORITY_SHIFT) | 3;
    pqAdd(&(procInfo->prq), process);
    process->debugEnv = (Envelope *)k_acquireMemoryBlock(memInfo, CRT_PID);
		process->state = NEW;

    // Fib Process
    process = &(procInfo->processes[FIRST_USER_PID + pidOffset++]); // Push process function address onto stack
    *(process->startLoc) = ((uint32_t) fibProcess);
    process->priority = (1 << KERN_PRIORITY_SHIFT) | 2;
    pqAdd(&(procInfo->prq), process);
    process->debugEnv = (Envelope *)k_acquireMemoryBlock(memInfo, CRT_PID);
		process->state = NEW;

    // Memory muncher Process
    process = &(procInfo->processes[FIRST_USER_PID + pidOffset++]); // Push process function address onto stack
    *(process->startLoc) = ((uint32_t) memoryMuncherProcess);
    process->priority = (1 << KERN_PRIORITY_SHIFT) | 1;
    pqAdd(&(procInfo->prq), process);
    process->debugEnv = (Envelope *)k_acquireMemoryBlock(memInfo, CRT_PID);
		process->state = NEW;

    // Release Process
    process = &(procInfo->processes[FIRST_USER_PID + pidOffset++]); // Push process function address onto stack
    *(process->startLoc) = ((uint32_t) releaseProcess);
    process->priority = (1 << KERN_PRIORITY_SHIFT) | 0;
    pqAdd(&(procInfo->prq), process);
    process->debugEnv = (Envelope *)k_acquireMemoryBlock(memInfo, CRT_PID);
		process->state = NEW;

    // TODO(shale): remove this assertion.
    if ((FIRST_USER_PID + pidOffset) >= STRESS_A_PID) {
            *((int *)-1) = 0;
    }
    // Stress Process A
    process = &(procInfo->processes[STRESS_A_PID]); // Push process function address onto stack
    *(process->startLoc) = ((uint32_t) stressAProcess);
    process->priority = (1 << KERN_PRIORITY_SHIFT) | 0;
    pqAdd(&(procInfo->prq), process);
    process->debugEnv = (Envelope *)k_acquireMemoryBlock(memInfo, CRT_PID);
    process->state = NEW;

    // Stress Process B
    process = &(procInfo->processes[STRESS_B_PID]); // Push process function address onto stack
    *(process->startLoc) = ((uint32_t) stressBProcess);
    process->priority = (1 << KERN_PRIORITY_SHIFT) | 0;
    pqAdd(&(procInfo->prq), process);
    process->debugEnv = (Envelope *)k_acquireMemoryBlock(memInfo, CRT_PID);
	process->state = NEW;

    // Stress Process C
    process = &(procInfo->processes[STRESS_C_PID]); // Push process function address onto stack
    *(process->startLoc) = ((uint32_t) stressCProcess);
    process->priority = (1 << KERN_PRIORITY_SHIFT) | 0;
    pqAdd(&(procInfo->prq), process);
    process->debugEnv = (Envelope *)k_acquireMemoryBlock(memInfo, CRT_PID);
    process->state = NEW;

    // Set Priority Process
    process = &(procInfo->processes[SET_PRIORITY_PID]); // Push process function address onto stack
    *(process->startLoc) = ((uint32_t) setPriorityProcess);
    process->priority = (1 << KERN_PRIORITY_SHIFT) | 0;
    pqAdd(&(procInfo->prq), process);
    process->debugEnv = (Envelope *)k_acquireMemoryBlock(memInfo, CRT_PID);
	process->state = NEW;

    procInfo->currentProcess = NULL;

    // Init UART keyboard global output data
    procInfo->uartOutputEnv = (Envelope *)k_acquireMemoryBlock(memInfo, CRT_PID);
	procInfo->uartOutputEnv->messageType = MT_CRT_WAKEUP;

    // Init UART keyboard global input data
    procInfo->currentEnv = (Envelope *)k_acquireMemoryBlock(memInfo, CRT_PID);
    procInfo->currentEnv->messageType = MT_KEYBOARD;

    crt_init(&(procInfo->crtData));
}


size_t writePCBState(char *buf, size_t bufLen, ProcState state) {
    switch (state) {
        case BLOCKED_MEMORY:
            return write_string(buf, bufLen, "Blocked on memory");
        case BLOCKED_MESSAGE:
            return write_string(buf, bufLen, "Blocked on message");
        case NEW:
            return write_string(buf, bufLen, "New");
        case READY:
            return write_string(buf, bufLen, "Ready");
        case RUNNING:
            return write_string(buf, bufLen, "Running");
        default:
            break;
    }

    return write_string(buf, bufLen, "???");
}

size_t writeProcessInfo(char *buf, size_t bufLen, PCB *pcb) {
    size_t i = 0;
    i += write_ansi_escape(buf+i, bufLen-i, 41);
    i += write_uint32(buf+i, bufLen-i, pcb->pid, 0);
    i += write_string(buf+i, bufLen-i, "$ Priority=");
    i += write_uint32(buf+i, bufLen-i, pcb->priority, 0);
    i += write_string(buf+i, bufLen-i, ", Status=");
    i += writePCBState(buf+i, bufLen-i, pcb->state);
    i += write_ansi_escape(buf+i, bufLen-i, 0);
    i += write_string(buf+i, bufLen-i, "\n");
    return i;
}

void k_processUartInput(ProcInfo *procInfo, MemInfo *memInfo) {
    uint32_t localReader = procInfo->readIndex;
    uint32_t localWriter = procInfo->writeIndex;

    while (procInfo->currentEnv != NULL && localReader != localWriter) {
        Envelope *kcdEnv = NULL;
        char new_char = procInfo->inputBuf[localReader];
        localReader = (localReader + 1) % UART_IN_BUF_SIZE;
        CRTData *crt = &(procInfo->crtData);

        if (new_char == '\r') {
            uint8_t i = 0;
            // Copy from crtData to our output envelope.
            for (i = 0; i < crt->lineBufLen; ++i) {
                procInfo->currentEnv->messageData[i] = crt->lineBuf[i];
            }

            // Append \n\0 to message.
            procInfo->currentEnv->messageData[crt->lineBufLen] = '\n';
            procInfo->currentEnv->messageData[(crt->lineBufLen)+1] = '\0';

            // Try to allocate a new envelope to send to KCD.
            kcdEnv = (Envelope *)k_acquireMemoryBlock(memInfo, KEYBOARD_PID);

            procInfo->currentEnv->messageType = MT_KEYBOARD;
            // copy_envelope will do nothing if kcdEnv is null
            copy_envelope(kcdEnv, procInfo->currentEnv);

            if (kcdEnv == NULL) {
                size_t i = 0;
                size_t bufLen = MESSAGEDATA_SIZE_BYTES - 1;
                i += write_ansi_escape(
                    procInfo->currentEnv->messageData+i,
                    bufLen-i,
                    31
                );
                i += write_string(
                    procInfo->currentEnv->messageData+i,
                    bufLen-i,
                    "System is out of memory. Please try again later.\n"
                );
                i += write_ansi_escape(
                    procInfo->currentEnv->messageData+i,
                    bufLen-i,
                    0
                );
                procInfo->currentEnv->messageData[i++] = '\0';
            }

            k_sendMessage(memInfo, procInfo, procInfo->currentEnv, CRT_PID, CRT_PID);
            procInfo->currentEnv = NULL;

            // If we're out of memory we will silently fail sending the message to KCD (it still shows on screen).
            if (kcdEnv != NULL) {
                k_sendMessage(memInfo, procInfo, kcdEnv, KEYBOARD_PID, KEYBOARD_PID);
            }

            kcdEnv = NULL;
            continue;
        } else if (new_char == SHOW_DEBUG_PROCESSES) {
            uint8_t i = 0;
            uint8_t bufLen = 0;
            char *buf = NULL;

            if (procInfo->debugSem > 0) {
                continue;
            }

            ++(procInfo->debugSem);

            i = 0;
            buf = procInfo->currentEnv->messageData;
            bufLen = MESSAGEDATA_SIZE_BYTES-1; // -1 for null byte

            i += write_ansi_escape(buf+i, bufLen-i, 41);
            i += write_string(buf+i, bufLen-i, "used mem = ");
            i += write_uint32(buf+i, bufLen-i, (memInfo->numSuccessfulAllocs-memInfo->numFreeCalls)*128, 2);
            i += write_string(buf+i, bufLen-i, " bytes");
            i += write_ansi_escape(buf+i, bufLen-i, 0);
            i += write_string(buf+i, bufLen-i, "\n");
            buf[i++] = '\0';

            procInfo->currentEnv->messageType = MT_KEYBOARD;
            k_sendMessage(memInfo, procInfo, procInfo->currentEnv, CRT_PID, CRT_PID);
            procInfo->currentEnv = NULL;
            buf = NULL;

            for (i = 0; i < NUM_PROCS; i++) {
                Envelope *tempEnvelope = NULL;
                uint32_t location = 0;
                PCB *pcb = &(procInfo->processes[i]);

                // Check if this is an unused process slot
                if (pcb->state == UNUSED || pcb->debugEnv == NULL) {
                    continue;
                }

                ++(procInfo->debugSem);

                tempEnvelope = pcb->debugEnv;
                location += writeProcessInfo(
                    tempEnvelope->messageData,
                    MESSAGEDATA_SIZE_BYTES - 1, // -1 for null byte
                    pcb
                );
                tempEnvelope->messageData[location++] = '\0';
                tempEnvelope->messageType = MT_DEBUG;
                k_sendMessage(memInfo, procInfo, tempEnvelope, CRT_PID, CRT_PID);
                tempEnvelope = NULL;
            }
            --(procInfo->debugSem);
            continue;
        } else {
            crt_pushUserByte(crt, new_char);
            continue;
        }
    }
    procInfo->readIndex = localReader;
}

void k_processUartOutput(ProcInfo *procInfo, MemInfo *memInfo) {
    LPC_UART_TypeDef *uart = (LPC_UART_TypeDef *)LPC_UART0;
    Envelope *temp = NULL;

    // NOTE(sanjay): these checks are sorted roughly in order of cheapness.

    // If we don't have our global envelope, we've already
    // pinged CRT proc, so give up.
    if (procInfo->uartOutputEnv == NULL) {
        return;
    }

    // If CRT proc is awake, then give up.
    if (procInfo->processes[CRT_PID].state == READY) {
        return;
    }

    // If CRT proc is asleep, but wouldn't be able to do anything anyways,
    // give up.
    if (!(uart->LSR & LSR_THRE)) {
        return;
    }

    // If CRT proc would be able to do something, but has nothing to send,
    // give up.
    if (!crt_hasOutByte(&(procInfo->crtData))) {
        return;
    }

    // If we don't have our global envelope, we've already
    // pinged CRT proc, so give up.
    if (procInfo->uartOutputEnv == NULL) {
        return;
    }

    // Otherwise, CRT proc is asleep, can print something,
    // and has something to print, so we should wake it up.
    temp = procInfo->uartOutputEnv;
    temp->messageType = MT_CRT_WAKEUP;
    procInfo->uartOutputEnv = NULL;
    k_sendMessage(memInfo, procInfo, temp, CRT_PID, CRT_PID);
}

uint32_t k_releaseProcessor(ProcInfo *procInfo, MemInfo *memInfo, MessageInfo *messageInfo, ClockInfo *clockInfo, ReleaseReason reason) {
    PCB *nextProc = NULL;
    ProcState oldState = NEW;

    // We need to set these three variables depending on the release reason
    ProcState targetState = READY;
    // We pull the next process to execute from the source queue
    PQ *srcQueue = NULL;
    // We push the currently executing process onto this queue
    PQ *dstQueue = NULL;

    k_processUartOutput(procInfo, memInfo);
    k_processUartInput(procInfo, memInfo);
    k_processDelayedMessages(messageInfo, procInfo, memInfo, clockInfo);

    switch (reason) {
        case OOM:
            srcQueue = &(procInfo->prq);
            dstQueue = &(procInfo->memq);
            targetState = BLOCKED_MEMORY;
            break;
        case YIELD:
        case CHANGED_PRIORITY:
        case MESSAGE_SENT:
        case MEMORY_FREED:
            srcQueue = &(procInfo->prq);
            dstQueue = &(procInfo->prq);
            targetState = READY;

            // If it was the null process that yielded, we don't add it to the ready queue.
            if (procInfo->currentProcess == procInfo->nullProcess) {
                dstQueue = NULL;
            }
            break;
        case MESSAGE_RECEIVE:
            if (procInfo->currentProcess->mqHead == NULL) {
                srcQueue = &(procInfo->prq);
                dstQueue = NULL;
                targetState = BLOCKED_MESSAGE;
            } else {
                srcQueue = &(procInfo->prq);
                dstQueue = &(procInfo->prq);
                targetState = READY;
            }
            break;
        default:
            break;
    }

    if (srcQueue != NULL && srcQueue->size > 0) {
        nextProc = pqTop(srcQueue);
        pqRemove(srcQueue, 0);
    } else {
        nextProc = procInfo->nullProcess;
    }

    if (procInfo->currentProcess != NULL) {
        // Save old process info
        procInfo->currentProcess->stack = (uint32_t *) __get_MSP();
        procInfo->currentProcess->state = targetState;
        if (dstQueue != NULL) {
            pqAdd(dstQueue, procInfo->currentProcess);
        }
    }

    oldState = nextProc->state;
    nextProc->state = RUNNING;
    procInfo->currentProcess = nextProc;
    __set_MSP((uint32_t) nextProc->stack);
    if (oldState == NEW) {
        __rte();
    }

    return 0;
}

/**
 * Behavior:
 *  Change specified process's priority (can't change null process's priority)
 *  If current process priority was changed to better priority, continue running
 *  Otherwise, if current process priority is worse than top priority in queue, preempt
 */
uint32_t k_setProcessPriority(ProcInfo *procInfo, MemInfo *memInfo, MessageInfo *messageInfo, ClockInfo *clockInfo, ProcId pid, uint8_t priority) {
    PCB *topProcess = NULL;
    PCB *modifiedProcess = NULL;
    uint32_t oldPriority = 0;
		uint32_t newPriority = 0;

    if (procInfo->currentProcess == NULL) {
        // TODO(sanjay): this error code seems a little inappropriate...
        return EINVAL;
    }

    modifiedProcess = k_getPCB(procInfo, pid);

    if (modifiedProcess == NULL) {
        return EINVAL;
    }

    // We don't allow changing the priority of the null process
    if (modifiedProcess == procInfo->nullProcess) {
        return EINVAL;
    }

    oldPriority = modifiedProcess->priority;
    newPriority =
        (priority & USER_PRIORITY_MASK) |
        (oldPriority & KERN_PRIORITY_MASK);
    modifiedProcess->priority = newPriority;

    // These are safe to call if the specified PCB is not in the particular
    // queue.
    pqChangedPriority(&(procInfo->prq), modifiedProcess);
    pqChangedPriority(&(procInfo->memq), modifiedProcess);

    // If we just improved our own priority, do not preempt.
    if (procInfo->currentProcess == modifiedProcess && oldPriority >= newPriority) {
        return SUCCESS;
    }

    // TODO(alex): Create function/struct to define comparison for priorities
    // (also use it in pqLess).

    // Otherwise, we'll preempt this process if the top process has a better
    // priority.
    topProcess = pqTop(&(procInfo->prq));
    if (topProcess->priority >= procInfo->currentProcess->priority) {
        return SUCCESS;
    }

    // Preempt the current process; note that the changed process won't
    // necessarily be the one to run
    k_releaseProcessor(procInfo, memInfo, messageInfo, clockInfo, CHANGED_PRIORITY);
    return SUCCESS;
}

int16_t k_getProcessPriority(ProcInfo *procInfo, ProcId pid) {
    uint8_t priority = 0;
    if (pid >= NUM_PROCS) {
        // TODO: Constantify
        return -1;
    }

    priority = (procInfo->processes[pid].priority & USER_PRIORITY_MASK);
    return (int16_t)(priority);
}

ProcId k_getPid(ProcInfo *procInfo) {
    return procInfo->currentProcess->pid;
}

PCB *k_getPCB(ProcInfo *procInfo, uint8_t pid) {
    PCB *pcb = NULL;

    if (pid >= NUM_PROCS) {
        return NULL;
    }

    pcb = &(procInfo->processes[pid]);

    if (pcb->state == UNUSED) {
        return NULL;
    }

    return pcb;
}
