#ifndef PQ_H
#define PQ_H

#include <stdint.h>

#include "heap.h"
#include "types.h"

void pqInit(PQ *q, PQEntry *heap, size_t pqStoreSize, storeIndexFunc fn);
PCB* pqTop(PQ *q);
uint32_t pqAdd(PQ *q, PCB *pcb);
PCB *pqRemove(PQ *q, size_t index);
void pqChangedPriority(PQ *q, PCB *pcb);

#endif // PQ_H
