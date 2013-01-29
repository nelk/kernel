
#include "prq.h"
#include "proc.h"

void prqInit(PRQ q, PCB **prStore, uint32_t prStoreSize) {
  q.heap = prStore;
  q.size = 0;
  q.cap = prStoreSize;
}

PCB* prqTop(PRQ q) {
  return q.heap[0];
}

uint32_t prqLess(PCB *a, PCB *b) {
  return a->priority < b->priority;
}

void prqUp(PRQ q, uint32_t j) {
  PCB *temp;
  while (1) {
    uint32_t i = (j - 1) / 2; // parent
    if (i == j || !prqLess(q.heap[j], q.heap[i])) {
      break;
    }
    temp = q.heap[i];
    q.heap[i] = q.heap[j];
    q.heap[j] = temp;
    j = i;
  }
}

void prqDown(PRQ q, uint32_t i) {
  uint32_t j1 = 0;
  uint32_t j = 0;
  uint32_t j2 = 0;
  uint32_t n = q.size;
  PCB *temp;

  while (1) {
    j1 = 2*i + 1;
    if (j1 >= n) {
      break;
    }
    j = j1; // left child
    j2 = j1 + 1;
    if (j2 < n && !prqLess(q.heap[j1], q.heap[j2])) {
      j = j2; // = 2*i + 2  // right child
    }
    if (!prqLess(q.heap[j], q.heap[i])) {
      break;
    }
    temp = q.heap[i];
    q.heap[i] = q.heap[j];
    q.heap[j] = temp;
    i = j;
  }
}

uint32_t prqAdd(PRQ q, PCB *pcb) {
  if (q.size >= q.cap) {
    return -1;
  }
  q.heap[q.size] = pcb;
  ++q.size;
  prqUp(q, q.size - 1);
  return 0;
}

PCB *prqRemove(PRQ q, uint32_t i) {
  PCB *removed = q.heap[i];
  q.heap[i] = q.heap[q.size - 1];
  //q.heap[q.size - 1] = removed;
  --q.size;
  prqDown(q, i);
  prqUp(q, i);
  return removed;
}


