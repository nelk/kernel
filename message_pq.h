#ifndef MESSAGE_PQ_H
#define MESSAGE_PQ_H

#include <stdint.h>

#include "heap.h"

struct Envelope;

typedef struct MessagePQ MessagePQ;
struct MessagePQ {
    heap storeMgr;
    Envelope **store;
    size_t size;
    size_t cap;

    uint32_t seq;
};

void mpqInit(MessagePQ *q, Envelope **pqStore, size_t pqStoreSize);
struct Envelope* mpqTop(MessagePQ *q);
uint32_t mpqAdd(MessagePQ *q, struct Envelope *pcb);
struct Envelope *mpqRemove(MessagePQ *q, size_t index);

#endif // MESSAGE_PQ_H
