#ifndef PROC_H
#define PROC_H

#include <stdint.h>

#include "types.h"

void k_initProcesses(MemInfo *memInfo, ProcInfo *procInfo);
uint32_t k_releaseProcessor(MemInfo *memInfo, ProcInfo *procInfo, MessageInfo *messageInfo, ClockInfo *clockInfo, ReleaseReason reason);
uint32_t k_setProcessPriority(MemInfo *memInfo, ProcInfo *procInfo, MessageInfo *messageInfo, ClockInfo *clockInfo,, ProcId pid, uint8_t priority);
int16_t k_getProcessPriority(ProcInfo *procInfo, ProcId pid);

#endif

