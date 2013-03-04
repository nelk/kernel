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

typedef ssize_t *(*storeIndexFunc)(struct PCB *);

typedef struct PQ PQ;
struct PQ {
  heap storeMgr;
  PQEntry *store;
  size_t size;
  size_t cap;

  uint32_t seq;

  storeIndexFunc getIndexInStore; // takes a PCB* and returns a ssize_t*
};

void pqInit(PQ *q, PQEntry *heap, size_t pqStoreSize);
struct PCB* pqTop(PQ *q);
uint32_t pqAdd(PQ *q, struct PCB *pcb);
struct PCB *pqRemove(PQ *q, size_t index);
void pqChangedPriority(PQ *q, struct PCB *pcb);

#endif // PQ_H
