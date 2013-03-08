#ifndef MESSAGE_H
#define MESSAGE_H

#include "common.h"
#include "types.h"
#include "message_pq.h"
#include "mem.h"
#include "proc.h"

void k_initMessages(MessageInfo *messageInfo, MemInfo *memInfo);

int8_t k_sendMessage(MemInfo *memInfo, ProcInfo *procInfo, Envelope *envelope, ProcId receiverPid, ProcId senderPid);

Envelope *k_receiveMessage(MessageInfo *messageInfo, MemInfo *memInfo, ProcInfo *procInfo, ClockInfo *clockInfo);

int8_t k_delayedSend(MessageInfo *messageInfo, MemInfo *memInfo, ProcInfo *procInfo, ClockInfo *clockInfo, ProcId pid, Envelope *envelope, uint32_t delay);

void k_processDelayedMessages(MessageInfo *messageInfo, ProcInfo *procInfo, MemInfo *memInfo, ClockInfo *clockInfo);

#endif
