
#ifndef PRQ_H
#define PRQ_H

#include <stdint.h>

struct PCB;

typedef struct PRQ PRQ;
struct PRQ {
  PCB **heap;
  uint32_t size;
  uint32_t cap;
};

void prqInit(PRQ q, PCB **prStore, uint32_t prStoreSize);
PCB* prqTop(PRQ);
uint32_t prqAdd(PRQ, PCB *);
PCB *prqRemove(PRQ, uint32_t);

#endif

