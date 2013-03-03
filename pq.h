#ifndef PQ_H
#define PQ_H

#include <stdint.h>

#include "heap.h"

struct PCB;

typedef struct PQEntry PQEntry;
struct PQEntry {
  struct PCB *pcb;
  uint32_t seqNumber;
};

typedef struct PQ PQ;
struct PQ {
  heap storeMgr;
  PQEntry *store;
  uint32_t size;
  uint32_t cap;

  uint32_t seq;
};

void pqInit(PQ *q, PQEntry *heap, uint32_t pqStoreSize);
struct PCB* pqTop(PQ *q);
uint32_t pqAdd(PQ *q, struct PCB *pcb);
struct PCB *pqRemove(PQ *q, uint32_t index);
struct PCB *pqRemoveByPid(PQ *q, uint32_t pid);

#endif // PQ_H
