
#include <stddef.h>
#include "prq.h"
#include "proc.h"

/*
 * Heap implementation ported from the Go language's standard library
 * http://tip.golang.org/src/pkg/container/heap/heap.go
 */

void prqInit(PRQ *q, PRQEntry *prStore, uint32_t prStoreSize) {
  q->heap = prStore;
  q->size = 0;
  q->cap = prStoreSize;
}

uint32_t prqNextSeq(PRQ *q) {
  uint32_t nextSeq = q->seq;
  ++(q->seq);
  return nextSeq;
}

PCB* prqTop(PRQ *q) {
  if (q->size == 0) {
    return NULL;
  }

  return q->heap[0].pcb;
}

uint32_t prqLess(PRQEntry a, PRQEntry b) {
  if (a.pcb->priority != b.pcb->priority) {
    return a.pcb->priority < b.pcb->priority;
  }

  if (a.seqNumber != b.seqNumber) {
    return a.seqNumber < b.seqNumber;
  }

  // This is a last-resort fallback. This case should never be triggered
  return a.pcb < b.pcb;
}

void prqUp(PRQ *q, uint32_t j) {
  PRQEntry temp;
  while (1) {
    uint32_t i = (j - 1) / 2; // parent
    if (i == j || !prqLess(q->heap[j], q->heap[i])) {
      break;
    }
    temp = q->heap[i];
    q->heap[i] = q->heap[j];
    q->heap[j] = temp;
    j = i;
  }
}

void prqDown(PRQ *q, uint32_t i) {
  uint32_t j1 = 0;
  uint32_t j = 0;
  uint32_t j2 = 0;
  uint32_t n = q->size;
  PRQEntry temp;

  while (1) {
    j1 = 2*i + 1;
    if (j1 >= n) {
      break;
    }
    j = j1; // left child
    j2 = j1 + 1;
    if (j2 < n && !prqLess(q->heap[j1], q->heap[j2])) {
      j = j2; // = 2*i + 2  // right child
    }
    if (!prqLess(q->heap[j], q->heap[i])) {
      break;
    }
    temp = q->heap[i];
    q->heap[i] = q->heap[j];
    q->heap[j] = temp;
    i = j;
  }
}

uint32_t prqAdd(PRQ *q, PCB *pcb) {
  PRQEntry newEntry;

  if (q->size >= q->cap) {
    return 1;
  }

  newEntry.pcb = pcb;
  newEntry.seqNumber = prqNextSeq(q);

  q->heap[q->size] = newEntry;
  ++(q->size);
  prqUp(q, q->size - 1);
  return 0;
}

PCB *prqRemove(PRQ *q, uint32_t i) {
  PCB *removed = q->heap[i].pcb;
  q->heap[i] = q->heap[q->size - 1];
  --(q->size);
  prqDown(q, i);
  prqUp(q, i);
  return removed;
}
