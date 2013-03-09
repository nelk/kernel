#include <stdint.h>
#include <stddef.h>

#include <LPC17xx.h>

#include "kernel_types.h"
#include "mem.h"
#include "message.h"
#include "proc.h"
#include "pq.h"
#include "timer.h"
#include "user.h"

extern void __rte(void);
extern void  __set_MSP(uint32_t);
extern uint32_t __get_MSP(void);

void processUartInput(ProcInfo *procInfo, MemInfo *memInfo);w

ssize_t *rqStoreIndexFunc(PCB *pcb) {
    return &(pcb->rqIndex);
}

ssize_t *memqStoreIndexFunc(PCB *pcb) {
    return &(pcb->memqIndex);
}

void k_initProcesses(ProcInfo *procInfo, MemInfo *memInfo) {
    ProcId i;
    int j;
    PCB *process = NULL;
    uint32_t *stack = NULL;

    procInfo->uartOutputComplete = 0;
    procInfo->uartOutputEnv = (Envelope *)k_acquireMemoryBlock(memInfo, CRT_PID);

    pqInit(&(procInfo->prq), procInfo->procQueue, NUM_PROCS, &rqStoreIndexFunc);
    pqInit(&(procInfo->memq), procInfo->memQueue, NUM_PROCS, &memqStoreIndexFunc);

    for (i = 0; i < NUM_PROCS; ++i) {
        process = &(procInfo->processes[i]);
        process->pid = i;
        process->state = NEW;
        process->mqHead = NULL;
        process->mqTail = NULL;
        // TODO(nelk): Assert that these memory blocks are contiguous
        // Stack grows backwards, not forwards. We allocate two memory blocks.
        k_acquireMemoryBlock(memInfo, PROC_ID_KERNEL);
        stack =
            (uint32_t *)(
                    (uint32_t)k_acquireMemoryBlock(memInfo, PROC_ID_KERNEL) + memInfo->blockSizeBytes
                    );

        if (!(((uint32_t)stack) & 0x04)) {
            --stack;
        }

        *(--stack) = 0x01000000; // <- does this work? (this is called XPsr)
        process->startLoc = --stack;

        for (j = 0; j < 6; j++) {
            *(--stack) = 0x0;
        }

        process->stack = stack;
    }

    // Null Process
    process = &(procInfo->processes[0]); // Push process function address onto stack
    *(process->startLoc) = ((uint32_t) nullProcess);
    process->priority = (2 << KERN_PRIORITY_SHIFT) | MAX_PRIORITY;
    procInfo->nullProcess = process;

    // Fun Process
    process = &(procInfo->processes[1]); // Push process function address onto stack
    *(process->startLoc) = ((uint32_t) funProcess);
    process->priority = (1 << KERN_PRIORITY_SHIFT) | 3;
    pqAdd(&(procInfo->prq), process);

    // Schizo Process
    process = &(procInfo->processes[2]); // Push process function address onto stack
    *(process->startLoc) = ((uint32_t) schizophrenicProcess);
    process->priority = (1 << KERN_PRIORITY_SHIFT) | 3;
    pqAdd(&(procInfo->prq), process);

    // fib Process
    process = &(procInfo->processes[3]); // Push process function address onto stack
    *(process->startLoc) = ((uint32_t) fibProcess);
    process->priority = (1 << KERN_PRIORITY_SHIFT) | 2;
    pqAdd(&(procInfo->prq), process);

    // memory muncher Process
    process = &(procInfo->processes[4]); // Push process function address onto stack
    *(process->startLoc) = ((uint32_t) memoryMuncherProcess);
    process->priority = (1 << KERN_PRIORITY_SHIFT) | 1;
    pqAdd(&(procInfo->prq), process);

    // release Process
    process = &(procInfo->processes[5]); // Push process function address onto stack
    *(process->startLoc) = ((uint32_t) releaseProcess);
    process->priority = (1 << KERN_PRIORITY_SHIFT) | 0;
    pqAdd(&(procInfo->prq), process);

    procInfo->currentProcess = NULL;


    // Init UART keyboard global input data
    procInfo->readIndex = 0;
    procInfo->writeIndex = 0;
    procInfo->inputBufOverflow = 0;
    procInfo->currentEnv = NULL;
    procInfo->currentEnvIndex = 0;
}

void k_processUartOutput(ProcInfo *procInfo, MemInfo *memInfo) {
    if (!procInfo->uartOutputComplete) {
        return;
    }

    k_sendMessage(memInfo, procInfo, procInfo->uartOutputEnv, CRT_PID, CRT_PID);
    procInfo->uartOutputComplete = 0;
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
    processUartInput(procInfo, memInfo);
    k_processDelayedMessages(messageInfo, procInfo, memInfo, clockInfo);

    switch (reason) {
        case MEMORY_FREED:
            srcQueue = &(procInfo->memq);
            dstQueue = &(procInfo->prq);
            targetState = READY;
            break;
        case OOM:
            srcQueue = &(procInfo->prq);
            dstQueue = &(procInfo->memq);
            targetState = BLOCKED;
            break;
        case YIELD:
        case CHANGED_PRIORITY:
        case MESSAGE_SENT:
            srcQueue = &(procInfo->prq);
            dstQueue = &(procInfo->prq);
            targetState = READY;

            // If it was the null process that yielded, we don't add it to the ready queue.
            if (procInfo->currentProcess == procInfo->nullProcess) {
                dstQueue = NULL;
            }
            break;
        case MESSAGE_RECEIVE:
            srcQueue = &(procInfo->prq);
            dstQueue = &(procInfo->prq);
            targetState = BLOCKED_MESSAGE;
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
    uint8_t oldPriority;

    if (procInfo->currentProcess == NULL) {
        // TODO(sanjay): this error code seems a little inappropriate...
        return EINVAL;
    }
    if (pid >= NUM_PROCS) {
        return EINVAL;
    }

    modifiedProcess = &(procInfo->processes[pid]);

    // We don't allow changing the priority of the null process
    if (modifiedProcess == procInfo->nullProcess) {
        return EINVAL;
    }

    oldPriority = modifiedProcess->priority;
    priority =
        (priority & USER_PRIORITY_MASK) |
        (oldPriority & KERN_PRIORITY_MASK);
    modifiedProcess->priority = priority;

    // These are safe to call if the specified PCB is not in the particular
    // queue.
    pqChangedPriority(&(procInfo->prq), modifiedProcess);
    pqChangedPriority(&(procInfo->memq), modifiedProcess);

    // If we just improved our own priority, do not preempt.
    if (procInfo->currentProcess == modifiedProcess && oldPriority >= priority) {
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
        return -1;
    }

    priority = (procInfo->processes[pid].priority & USER_PRIORITY_MASK);
    return (int16_t)(priority);
}

ProcId k_getPid(ProcInfo *procInfo) {
    return procInfo->currentProcess->pid;

void processUartInput(ProcInfo *procInfo, MemInfo *memInfo) {
    ProcId keyboardPid;
    uint32_t localReader = procInfo->readIndex;
    uint32_t localWriter = procInfo->writeIndex;

    while (procInfo->currentEnv != NULL && localReader != localWriter) {
        char new_char = procInfo->inputBuf[localReader];
        localReader = (localReader + 1) % UART_IN_BUF_SIZE;

        if (new_char == '\n') {
            if (procInfo->inputBufOverflow) {
                // Reuse current envelope
                procInfo->currentEnvIndex = 0;
                procInfo->inputBufOverflow = 0;
                continue;
            }
            k_sendMessage(memInfo, procInfo, procInfo->currentEnv, keyboardPid, keyboardPid); // No preemption
            procInfo->currentEnv = (Envelope *)k_acquire_memory_block(memInfo, keyboardPid);
            procInfo->currentEnvIndex = 0;
            continue;
        }
        if (procInfo->currentEnvIndex >= 96) { // Constantified
            procInfo->inputBufOverflow = 1;
            continue;
        }
        procInfo->currentEnv->messageBody[procInfo->currentEnvIndex] = new_char;
        ++currentEnvIndex;
    }
    procInfo->readIndex = localReader;
}

