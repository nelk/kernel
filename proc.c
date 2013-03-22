#include <stdint.h>
#include <stddef.h>

#include <LPC17xx.h>

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
    ProcId i;
    int j;
    PCB *process = NULL;
    uint32_t *stack = NULL;
    uint8_t pidOffset = 0;

    pqInit(&(procInfo->prq), procInfo->procQueue, NUM_PROCS, &rqStoreIndexFunc);
    pqInit(&(procInfo->memq), procInfo->memQueue, NUM_PROCS, &memqStoreIndexFunc);

    for (i = 0; i < NUM_PROCS; ++i) {
        uint32_t tempStack = 0;
        process = &(procInfo->processes[i]);
        process->pid = i;
        process->state = NEW;
        process->mqHead = NULL;
        process->mqTail = NULL;
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

    // Clock Process
    process = &(procInfo->processes[CLOCK_PID]); // Push process function address onto stack
    *(process->startLoc) = ((uint32_t) clockProcess);
    process->priority = (0 << KERN_PRIORITY_SHIFT) | 2;
    pqAdd(&(procInfo->prq), process);

    // CRT Process
    process = &(procInfo->processes[CRT_PID]); // Push process function address onto stack
    *(process->startLoc) = ((uint32_t) crt_proc);
    process->priority = (0 << KERN_PRIORITY_SHIFT) | 0;
    pqAdd(&(procInfo->prq), process);

    // Keyboard Process
    process = &(procInfo->processes[KEYBOARD_PID]); // Push process function address onto stack
    *(process->startLoc) = ((uint32_t) uart_keyboard_proc);
    process->priority = (0 << KERN_PRIORITY_SHIFT) | 1;
    pqAdd(&(procInfo->prq), process);

    // Fun Process
    process = &(procInfo->processes[FIRST_USER_PID + pidOffset++]); // Push process function address onto stack
    *(process->startLoc) = ((uint32_t) funProcess);
    process->priority = (1 << KERN_PRIORITY_SHIFT) | 3;
    pqAdd(&(procInfo->prq), process);

    // Schizo Process
    process = &(procInfo->processes[FIRST_USER_PID + pidOffset++]); // Push process function address onto stack
    *(process->startLoc) = ((uint32_t) schizophrenicProcess);
    process->priority = (1 << KERN_PRIORITY_SHIFT) | 3;
    pqAdd(&(procInfo->prq), process);

    // Fib Process
    process = &(procInfo->processes[FIRST_USER_PID + pidOffset++]); // Push process function address onto stack
    *(process->startLoc) = ((uint32_t) fibProcess);
    process->priority = (1 << KERN_PRIORITY_SHIFT) | 2;
    pqAdd(&(procInfo->prq), process);

    // Memory muncher Process
    process = &(procInfo->processes[FIRST_USER_PID + pidOffset++]); // Push process function address onto stack
    *(process->startLoc) = ((uint32_t) memoryMuncherProcess);
    process->priority = (1 << KERN_PRIORITY_SHIFT) | 1;
    pqAdd(&(procInfo->prq), process);

    // Release Process
    process = &(procInfo->processes[FIRST_USER_PID + pidOffset++]); // Push process function address onto stack
    *(process->startLoc) = ((uint32_t) releaseProcess);
    process->priority = (1 << KERN_PRIORITY_SHIFT) | 0;
    pqAdd(&(procInfo->prq), process);

    // TODO(shale): Assert these pids match the spec.

    // Stress Process A
    process = &(procInfo->processes[FIRST_USER_PID + pidOffset++]); // Push process function address onto stack
    *(process->startLoc) = ((uint32_t) stressAProcess);
    process->priority = (1 << KERN_PRIORITY_SHIFT) | 0;
    pqAdd(&(procInfo->prq), process);

    // Stress Process B
    process = &(procInfo->processes[FIRST_USER_PID + pidOffset++]); // Push process function address onto stack
    *(process->startLoc) = ((uint32_t) stressBProcess);
    process->priority = (1 << KERN_PRIORITY_SHIFT) | 0;
    pqAdd(&(procInfo->prq), process);

    // Stress Process C
    process = &(procInfo->processes[FIRST_USER_PID + pidOffset++]); // Push process function address onto stack
    *(process->startLoc) = ((uint32_t) stressCProcess);
    process->priority = (1 << KERN_PRIORITY_SHIFT) | 0;
    pqAdd(&(procInfo->prq), process);

    procInfo->currentProcess = NULL;

    // Init UART keyboard global output data
    procInfo->uartOutputPending = 0;
    procInfo->uartOutputEnv = (Envelope *)k_acquireMemoryBlock(memInfo, CRT_PID);

    // Init UART keyboard global input data
    procInfo->readIndex = 0;
    procInfo->writeIndex = 0;
    procInfo->inputBufOverflow = 0;
    procInfo->currentEnv = (Envelope *)k_acquireMemoryBlock(memInfo, KEYBOARD_PID);
    procInfo->currentEnvIndex = 0;
}

void k_processUartInput(ProcInfo *procInfo, MemInfo *memInfo) {
    uint32_t localReader = procInfo->readIndex;
    uint32_t localWriter = procInfo->writeIndex;

    if (procInfo->currentEnv == NULL) {
        procInfo->currentEnv = (Envelope *)k_acquireMemoryBlock(memInfo, KEYBOARD_PID);
    }
    while (procInfo->currentEnv != NULL && localReader != localWriter) {
        char new_char = procInfo->inputBuf[localReader];
        localReader = (localReader + 1) % UART_IN_BUF_SIZE;

        if (new_char == '\r') {
            if (procInfo->inputBufOverflow) {
                // Reuse current envelope
                procInfo->currentEnvIndex = 0;
                procInfo->inputBufOverflow = 0;
                continue;
            }

            // Append \r\n\0 to message.
            procInfo->currentEnv->messageData[procInfo->currentEnvIndex++] = '\r';
            procInfo->currentEnv->messageData[procInfo->currentEnvIndex++] = '\n';
            procInfo->currentEnv->messageData[procInfo->currentEnvIndex++] = '\0';

            k_sendMessage(memInfo, procInfo, procInfo->currentEnv, KEYBOARD_PID, KEYBOARD_PID); // No preemption
            procInfo->currentEnv = (Envelope *)k_acquireMemoryBlock(memInfo, KEYBOARD_PID);
            procInfo->currentEnvIndex = 0;
            continue;
        }

        // Write new character into message
        procInfo->currentEnv->messageData[procInfo->currentEnvIndex] = new_char;

        // If first character in message.
        if (procInfo->currentEnvIndex == 0) {
            switch (new_char) {
                case SHOW_DEBUG_PROCESSES:
                    k_sendMessage(memInfo, procInfo, procInfo->currentEnv, KEYBOARD_PID, KEYBOARD_PID);
                    procInfo->currentEnv = (Envelope *)k_acquireMemoryBlock(memInfo, KEYBOARD_PID);
                    procInfo->currentEnvIndex = 0;
                    continue;
                default:
                    break;
            }
        }
        if (procInfo->currentEnvIndex >= MESSAGEDATA_SIZE_BYTES - 3) { // -3 for \r\n\0
            procInfo->inputBufOverflow = 1;
            continue;
        }
        // Increment index in message
        ++(procInfo->currentEnvIndex);
    }
    procInfo->readIndex = localReader;
}

void k_processUartOutput(ProcInfo *procInfo, MemInfo *memInfo) {
    LPC_UART_TypeDef *uart = (LPC_UART_TypeDef *)LPC_UART0;
    Envelope *temp = NULL;

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
    if (!(procInfo->uartOutputPending)) {
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
        case MEMORY_FREED:
            srcQueue = &(procInfo->memq);
            dstQueue = &(procInfo->prq);
            targetState = READY;
            break;
        case OOM:
            srcQueue = &(procInfo->prq);
            dstQueue = &(procInfo->memq);
            targetState = BLOCKED_MEMORY;
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
}


