#ifndef PRQ_H
#define PRQ_H

#include <stdint.h>

#include "proc.h"

typedef struct PQEntry PQEntry;
struct PQEntry {
    PCB *pcb;
    uint32_t seqNumber;
};

typedef struct PQ PQ;
struct PQ {
  PQEntry *heap;
  uint32_t size;
  uint32_t cap;
  uint32_t seq;
};

void pqInit(PQ *q, PQEntry *heap, uint32_t pqStoreSize);
PCB* pqTop(PQ *q);
uint32_t pqAdd(PQ *q, PCB *pcb);
PCB *pqRemove(PQ *q, uint32_t index);

#endif
