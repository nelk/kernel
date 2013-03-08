#include <stdint.h>
#include <stddef.h>

#include <LPC17xx.h>
#include "mem.h"
#include "proc.h"
#include "pq.h"
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

void k_initProcesses(MemInfo *memInfo, ProcInfo *procInfo) {
    ProcId i;
    int j;
    PCB *process = NULL;
    uint32_t *stack = NULL;

    pqInit(&(procInfo->prq), procInfo->procQueue, NUM_PROCS, &rqStoreIndexFunc);
    pqInit(&(procInfo->memq), procInfo->memQueue, NUM_PROCS, &memqStoreIndexFunc);

    for (i = 0; i < NUM_PROCS; ++i) {
        process = &(procInfo->processes[i]);
        process->pid = i;
        process->state = NEW;
        process->messageQueue = NULL;
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
    process->priority = 4;
    procInfo->nullProcess = process;

    // Fun Process
    process = &(procInfo->processes[1]); // Push process function address onto stack
    *(process->startLoc) = ((uint32_t) funProcess);
    process->priority = 3;
    pqAdd(&(procInfo->prq), process);

    // Schizo Process
    process = &(procInfo->processes[2]); // Push process function address onto stack
    *(process->startLoc) = ((uint32_t) schizophrenicProcess);
    process->priority = 3;
    pqAdd(&(procInfo->prq), process);

    // fib Process
    process = &(procInfo->processes[3]); // Push process function address onto stack
    *(process->startLoc) = ((uint32_t) fibProcess);
    process->priority = 2;
    pqAdd(&(procInfo->prq), process);

    // memory muncher Process
    process = &(procInfo->processes[4]); // Push process function address onto stack
    *(process->startLoc) = ((uint32_t) memoryMuncherProcess);
    process->priority = 1;
    pqAdd(&(procInfo->prq), process);

    // release Process
    process = &(procInfo->processes[5]); // Push process function address onto stack
    *(process->startLoc) = ((uint32_t) releaseProcess);
    process->priority = 0;
    pqAdd(&(procInfo->prq), process);

    procInfo->currentProcess = NULL;
}

uint32_t k_releaseProcessor(ProcInfo *k_procInfo, ReleaseReason reason) {
    PCB *nextProc = NULL;
    ProcState oldState = NEW;

    // We need to set these three variables depending on the release reason
    ProcState targetState = READY;
    // We pull the next process to execute from the source queue
    PQ *srcQueue = NULL;
    // We push the currently executing process onto this queue
    PQ *dstQueue = NULL;

    // TODO - delayed send check

    switch (reason) {
        case MEMORY_FREED:
            srcQueue = &(k_procInfo->memq);
            dstQueue = &(k_procInfo->prq);
            targetState = READY;
            break;
        case OOM:
            srcQueue = &(k_procInfo->prq);
            dstQueue = &(k_procInfo->memq);
            targetState = BLOCKED;
            break;
        case YIELD:
        case CHANGED_PRIORITY:
        case MESSAGE_SENT:
            srcQueue = &(k_procInfo->prq);
            dstQueue = &(k_procInfo->prq);
            targetState = READY;

            // If it was the null process that yielded, we don't add it to the ready queue.
            if (k_procInfo->currentProcess == k_procInfo->nullProcess) {
                dstQueue = NULL;
            }
            break;
        case MESSAGE_RECEIVE:
            srcQueue = &(k_procInfo->prq);
            dstQueue = &(k_procInfo->prq);
            targetState = BLOCKED_MESSAGE;
            break;
        default:
            break;
    }

    if (srcQueue != NULL && srcQueue->size > 0) {
        nextProc = pqTop(srcQueue);
        pqRemove(srcQueue, 0);
    } else {
        nextProc = k_procInfo->nullProcess;
    }

    if (k_procInfo->currentProcess != NULL) {
        // Save old process info
        k_procInfo->currentProcess->stack = (uint32_t *) __get_MSP();
        k_procInfo->currentProcess->state = targetState;
        if (dstQueue != NULL) {
            pqAdd(dstQueue, k_procInfo->currentProcess);
        }
    }

    oldState = nextProc->state;
    nextProc->state = RUNNING;
    k_procInfo->currentProcess = nextProc;
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
uint32_t k_setProcessPriority(ProcInfo *procInfo, ProcId pid, uint8_t priority) {
    PCB *topProcess = NULL;
    PCB *modifiedProcess = NULL;
    uint8_t oldPriority;

    if (procInfo->currentProcess == NULL) {
        return 1;
    }
    if (pid >= NUM_PROCS) {
        return 1;
    }
    if (priority >= MAX_PRIORITY) { // We are not allowing any process to be set to the worst priority (The null process begins at the worst priority).
        return 2;
    }

    if (pid == procInfo->currentProcess->pid) {
        oldPriority = procInfo->currentProcess->priority;
        procInfo->currentProcess->priority = priority;
        if (oldPriority <= priority) { // If we made the priority of this process better, don't preempt it.
            return 0;
        }
    } else {
        modifiedProcess = &(procInfo->processes[pid]);
        if (modifiedProcess == procInfo->nullProcess) { // We don't allow changing the priority of the null process
            return 1;
        }
        modifiedProcess->priority = priority;
        pqChangedPriority(&(procInfo->prq), modifiedProcess);
        pqChangedPriority(&(procInfo->memq), modifiedProcess);
    }

    // TODO(alex): Create function/struct to define comparison for priorities (also use it in pqLess).
    topProcess = pqTop(&(procInfo->prq)); // We'll preempt this process if the top process has a better priority.
    if (topProcess->priority >= procInfo->currentProcess->priority) {
        return 0;
    }

    // Preempt the current process; note that the changed process won't necessarily be the one to run
    k_releaseProcessor(procInfo, CHANGED_PRIORITY);
		
		//TODO (shale): make this an enum
		return 0;
}

int16_t k_getProcessPriority(ProcInfo *procInfo, ProcId pid) {
    if (pid >= NUM_PROCS) {
        return -1;
    }

    return (int16_t)(procInfo->processes[pid].priority);
}

