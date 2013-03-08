#ifndef MESSAGE_H
#define MESSAGE_H

#include "types.h"
#include "message_pq.h"
#include "mem.h"
#include "proc.h"

void k_initMessages(MemInfo *memInfo, MessageInfo *messageInfo);

int8_t k_sendMessage(MemInfo *memInfo, ProcInfo *procInfo, ProcId pid, Envelope *envelope);

Envelope *k_receiveMessage(MemInfo *memInfo, ProcInfo *procInfo, ProcId *senderPid);

int8_t k_delayedSend(MemInfo *memInfo, MessageInfo *messageInfo, ProcInfo *procInfo, ProcId pid, Envelope *envelope, uint32_t delay);

#endif
