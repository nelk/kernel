#ifndef PRQ_H
#define PRQ_H

#include <stdint.h>

#include "proc.h"

typedef struct PRQ PRQ;
struct PRQ {
  PCB **heap;
  uint32_t size;
  uint32_t cap;
};

void prqInit(PRQ *q, PCB **prStore, uint32_t prStoreSize);
PCB* prqTop(PRQ *q);
uint32_t prqAdd(PRQ *q, PCB *pcb);
PCB *prqRemove(PRQ *q, uint32_t index);

#endif
