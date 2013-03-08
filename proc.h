#ifndef PROC_H
#define PROC_H

#include <stdint.h>

#include "mem.h"
#include "message.h"
#include "pq.h"
#include "kernel_types.h"

void k_initProcesses(MemInfo *memInfo, ProcInfo *procInfo);
uint32_t k_releaseProcessor(ProcInfo *procInfo, ReleaseReason reason);
uint32_t k_setProcessPriority(ProcInfo *procInfo, ProcId pid, uint8_t priority);
int16_t k_getProcessPriority(ProcInfo *procInfo, ProcId pid);

#endif

