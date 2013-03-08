#include <stddef.h>

#include "message_pq.h"
#include "message.h"

void mpqSwap(void *vCtx, size_t i, size_t j);
uint8_t mpqLess(void *vCtx, size_t i, size_t j);


void mpqInit(MessagePQ *q, Envelope **mpqStore, size_t pqStoreSize) {
    q->store = mpqStore;
    q->size = 0;
    q->cap = pqStoreSize;

    heapZero(&(q->storeMgr));
    heapSetLessFn(&(q->storeMgr), &mpqLess);
    heapSetSwapFn(&(q->storeMgr), &mpqSwap);
    heapSetContext(&(q->storeMgr), q);
}

uint32_t mpqNextSeq(MessagePQ *q) {
    uint32_t nextSeq = q->seq;
    ++(q->seq);
    return nextSeq;
}

Envelope* mpqTop(MessagePQ *q) {
    if (q->size == 0) {
        return NULL;
    }

    return q->store[0];
}

uint8_t mpqLess(void *vCtx, size_t i, size_t j) {
    MessagePQ *ctx = (MessagePQ *)vCtx;
    Envelope *lhs = ctx->store[i];
    Envelope *rhs = ctx->store[j];

    return lhs->header[SEND_TIME] < rhs->header[SEND_TIME];
}

void mpqSwap(void *vCtx, size_t i, size_t j) {
    MessagePQ *ctx = (MessagePQ *)vCtx;
    Envelope *temp = ctx->store[i];
    ctx->store[i] = ctx->store[j];
    ctx->store[j] = temp;

    temp = ctx->store[i];
}

uint32_t mpqAdd(MessagePQ *q, Envelope *env) {
    if (q->size >= q->cap) {
        return 1;
    }

    q->store[q->size] = env;
    ++(q->size);
    heapAdd(&(q->storeMgr));
    return 0;
}

Envelope *mpqRemove(MessagePQ *q, size_t i) {
    Envelope *removing = NULL;
    heapRemove(&(q->storeMgr), i);
    --(q->size);
    removing = q->store[q->size];

    return removing;
}

