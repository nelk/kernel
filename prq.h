#ifndef PRQ_H
#define PRQ_H

#include <stdint.h>

typedef struct PCB PCB;

typedef struct PRQEntry PRQEntry;
struct PRQEntry {
    PCB *pcb;
    uint32_t seqNumber;
};

typedef struct PRQ PRQ;
struct PRQ {
  PRQEntry *heap;
  uint32_t size;
  uint32_t cap;
  uint32_t seq;
};

void prqInit(PRQ *q, PRQEntry *heap, uint32_t prStoreSize);
PCB* prqTop(PRQ *q);
uint32_t prqAdd(PRQ *q, PCB *pcb);
PCB *prqRemove(PRQ *q, uint32_t index);

#endif
