
#include <stddef.h>
#include "pq.h"
#include "proc.h"

/*
 * Heap implementation ported from the Go language's standard library
 * http://tip.golang.org/src/pkg/container/heap/heap.go
 */

void pqInit(PQ *q, PQEntry *pqStore, uint32_t pqStoreSize) {
  q->heap = pqStore;
  q->size = 0;
  q->cap = pqStoreSize;
}

uint32_t pqNextSeq(PQ *q) {
  uint32_t nextSeq = q->seq;
  ++(q->seq);
  return nextSeq;
}

PCB* pqTop(PQ *q) {
  if (q->size == 0) {
    return NULL;
  }

  return q->heap[0].pcb;
}

uint32_t pqLess(PQEntry a, PQEntry b) {
  if (a.pcb->priority != b.pcb->priority) {
    return a.pcb->priority < b.pcb->priority;
  }

  if (a.seqNumber != b.seqNumber) {
    return a.seqNumber < b.seqNumber;
  }

  // This is a last-resort fallback. This case should never be triggered
  return a.pcb < b.pcb;
}

void pqUp(PQ *q, int32_t j) {
  PQEntry temp;
  while (1) {
    int32_t i = (j - 1) / 2; // parent
    if (i == j || !pqLess(q->heap[j], q->heap[i])) {
      break;
    }
    temp = q->heap[i];
    q->heap[i] = q->heap[j];
    q->heap[j] = temp;
    j = i;
  }
}

void pqDown(PQ *q, int32_t i) {
  int32_t j1 = 0;
  int32_t j = 0;
  int32_t j2 = 0;
  int32_t n = q->size;
  PQEntry temp;

  while (1) {
    j1 = 2*i + 1;
    if (j1 >= n) {
      break;
    }
    j = j1; // left child
    j2 = j1 + 1;
    if (j2 < n && !pqLess(q->heap[j1], q->heap[j2])) {
      j = j2; // = 2*i + 2  // right child
    }
    if (!pqLess(q->heap[j], q->heap[i])) {
      break;
    }
    temp = q->heap[i];
    q->heap[i] = q->heap[j];
    q->heap[j] = temp;
    i = j;
  }
}

uint32_t pqAdd(PQ *q, PCB *pcb) {
  PQEntry newEntry;

  if (q->size >= q->cap) {
    return 1;
  }

  newEntry.pcb = pcb;
  newEntry.seqNumber = pqNextSeq(q);

  q->heap[q->size] = newEntry;
  ++(q->size);
  pqUp(q, q->size - 1);
  return 0;
}

PCB *pqRemove(PQ *q, uint32_t i) {
  PCB *removed = q->heap[i].pcb;
  q->heap[i] = q->heap[q->size - 1];
  --(q->size);
  pqDown(q, i);
  pqUp(q, i);
  return removed;
}
