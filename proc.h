#ifndef PROC_H
#define PROC_H

#include <stdint.h>

#include "mem.h"
#include "message.h"
#include "pq.h"
#include "kernel_types.h"

void k_initProcesses(ProcInfo *procInfo, MemInfo *memInfo);
uint32_t k_releaseProcessor(ProcInfo *procInfo, MemInfo *memInfo, MessageInfo *messageInfo, ClockInfo *clockInfo, ReleaseReason reason);
uint32_t k_setProcessPriority(ProcInfo *procInfo, MemInfo *memInfo, MessageInfo *messageInfo, ClockInfo *clockInfo, ProcId pid, uint8_t priority);
int16_t k_getProcessPriority(ProcInfo *procInfo, ProcId pid);
ProcId k_getPid(ProcInfo *procInfo);
PCB *k_getPCB(ProcInfo *procInfo, uint8_t pid);


#endif

