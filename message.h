#ifndef MESSAGE_H
#define MESSAGE_H

#include "common.h"
#include "types.h"
#include "message_pq.h"
#include "mem.h"
#include "proc.h"



enum EnvelopeHeaderData {
    NEXT_ENVELOPE,
    SEND_TIME
};
typedef enum EnvelopeHeaderData EnvelopeHeaderData;

void k_initMessages(MemInfo *memInfo, MessageInfo *messageInfo);

int8_t k_sendMessage(MemInfo *memInfo, ProcInfo *procInfo, Envelope *envelope, ProcId receiverPid, ProcId senderPid);

Envelope *k_receiveMessage(MemInfo *memInfo, ProcInfo *procInfo, MessageInfo *messageInfo, ClockInfo *clockInfo);

int8_t k_delayedSend(MemInfo *memInfo, MessageInfo *messageInfo, ProcInfo *procInfo, ProcId pid, Envelope *envelope, uint32_t delay);

#endif
