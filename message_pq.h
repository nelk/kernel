#ifndef MESSAGE_PQ_H
#define MESSAGE_PQ_H

#include <stdint.h>

#include "heap.h"
#include "types.h"
#include "message.h"



void mpqInit(MessagePQ *q, Envelope **pqStore, size_t pqStoreSize);
struct Envelope* mpqTop(MessagePQ *q);
uint32_t mpqAdd(MessagePQ *q, struct Envelope *pcb);
struct Envelope *mpqRemove(MessagePQ *q, size_t index);

#endif // MESSAGE_PQ_H
