#ifndef PQ_H
#define PQ_H

#include <stdint.h>

#include "heap.h"
#include "kernel_types.h"

uint32_t pqAdd(PQ *q, PCB *pcb);
void pqChangedPriority(PQ *q, PCB *pcb);
void pqInit(PQ *q, PQEntry *heap, size_t pqStoreSize, storeIndexFunc fn);
PCB *pqRemove(PQ *q, size_t index);
PCB *pqTop(PQ *q);

#endif // PQ_H
