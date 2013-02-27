#include <stddef.h>
#include "pq.h"
#include "proc.h"

void pqSwap(void *vCtx, size_t i, size_t j);
uint8_t pqLess(void *vCtx, size_t i, size_t j);

void pqInit(PQ *q, PQEntry *pqStore, uint32_t pqStoreSize) {
  q->store = pqStore;
  q->size = 0;
  q->cap = pqStoreSize;

  heapZero(&(q->storeMgr));
  heapSetLessFn(&(q->storeMgr), &pqLess);
  heapSetSwapFn(&(q->storeMgr), &pqSwap);
  heapSetContext(&(q->storeMgr), q);
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

  return q->store[0].pcb;
}

uint8_t pqLess(void *vCtx, size_t i, size_t j) {
  PQ *ctx = (PQ *)vCtx;
  PQEntry lhs = ctx->store[i];
  PQEntry rhs = ctx->store[j];

  if (lhs.pcb->priority != rhs.pcb->priority) {
    return lhs.pcb->priority < rhs.pcb->priority;
  }

  if (lhs.seqNumber != rhs.seqNumber) {
    return lhs.seqNumber < rhs.seqNumber;
  }

  // This is a last-resort fallback. This case should never be triggered
  return lhs.pcb < rhs.pcb;
}

void pqSwap(void *vCtx, size_t i, size_t j) {
  PQ *ctx = (PQ *)vCtx;
  PQEntry temp = ctx->store[i];
  ctx->store[i] = ctx->store[j];
  ctx->store[j] = temp;
}

uint32_t pqAdd(PQ *q, PCB *pcb) {
  PQEntry newEntry;

  if (q->size >= q->cap) {
    return 1;
  }

  newEntry.pcb = pcb;
  newEntry.seqNumber = pqNextSeq(q);

  q->store[q->size] = newEntry;
  ++(q->size);
  heapAdd(&(q->storeMgr));
  return 0;
}

PCB *pqRemove(PQ *q, uint32_t i) {
  heapRemove(&(q->storeMgr), i);
  --(q->size);
  return q->store[q->size].pcb;
}
