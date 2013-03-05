#include <stddef.h>
#include "pq.h"
#include "proc.h"

void pqSwap(void *vCtx, size_t i, size_t j);
uint8_t pqLess(void *vCtx, size_t i, size_t j);

void pqInit(PQ *q, PQEntry *pqStore, size_t pqStoreSize, storeIndexFunc fn) {
    q->store = pqStore;
    q->size = 0;
    q->cap = pqStoreSize;
    q->getIndexInStore = fn;

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

    *(ctx->getIndexInStore(temp.pcb)) = j;
    temp = ctx->store[i];
    *(ctx->getIndexInStore(temp.pcb)) = i;
}

uint32_t pqAdd(PQ *q, PCB *pcb) {
    PQEntry newEntry;

    if (q->size >= q->cap) {
        return 1;
    }

    *(q->getIndexInStore(pcb)) = q->size;

    newEntry.pcb = pcb;
    newEntry.seqNumber = pqNextSeq(q);

    q->store[q->size] = newEntry;
    ++(q->size);
    heapAdd(&(q->storeMgr));
    return 0;
}

PCB *pqRemove(PQ *q, size_t i) {
    PCB *removing = NULL;
    heapRemove(&(q->storeMgr), i);
    --(q->size);
    removing = q->store[q->size].pcb;

    *(q->getIndexInStore(removing)) = -1;

    return removing;
}

void pqChangedPriority(PQ *q, struct PCB *pcb) {
    ssize_t index = -1;
    PQEntry updatingEntry;

    assert(q->getIndexInStore != NULL);
    index = *(q->getIndexInStore(pcb));

    if (index == -1) {
        return;
    }

    assert(index < q->size);
    updatingEntry = q->store[index];

    assert(updatingEntry.pcb == pcb);
    updatingEntry.seqNumber = pqNextSeq(q);
    heapInvalidate(&(q->storeMgr), index);
}

