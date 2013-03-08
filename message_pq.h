#ifndef MESSAGE_PQ_H
#define MESSAGE_PQ_H

#include <stdint.h>

#include "heap.h"
#include "kernel_types.h"
#include "message.h"



void mpqInit(MessagePQ *q, Envelope **pqStore, size_t pqStoreSize);
Envelope* mpqTop(MessagePQ *q);
uint32_t mpqAdd(MessagePQ *q, Envelope *env);
Envelope *mpqRemove(MessagePQ *q, size_t index);

#endif // MESSAGE_PQ_H
