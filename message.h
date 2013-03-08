#ifndef MESSAGE_H
#define MESSAGE_H

#include "common.h"
#include "types.h"
#include "message_pq.h"
#include "mem.h"
#include "proc.h"

void k_initMessages(MemInfo *memInfo, MessageInfo *messageInfo);

int8_t k_sendMessage(MemInfo *memInfo, ProcInfo *procInfo, Envelope *envelope, ProcId receiverPid, ProcId senderPid);

Envelope *k_receiveMessage(MemInfo *memInfo, ProcInfo *procInfo, MessageInfo *messageInfo, ClockInfo *clockInfo);

int8_t k_delayedSend(MemInfo *memInfo, ProcInfo *procInfo, MessageInfo *messageInfo, ClockInfo *clockInfo, ProcId pid, Envelope *envelope, uint32_t delay);

#endif
