#ifndef MESSAGE_H
#define MESSAGE_H

#include "common.h"
#include "message.h"
#include "message_pq.h"
#include "mem.h"
#include "proc.h"

// TODO(alex) - move Envelope to user-facing header (user_message.h) or aggregate user-facing types header.
typedef struct Envelope Envelope;
struct Envelope {
    uint32_t header[3];

    // User Data
    uint32_t senderPid;
    uint32_t destPid;
    uint32_t messageType;
    char messageData[BLOCKSIZE_BYTES - 6*sizeof(uint32_t)];
};

typedef struct MessageInfo MessageInfo;
struct MessageInfo {
    MessagePQ mpq; // Delayed Message PQ
    Envelope *messageStore[500];
};

enum EnvelopeHeaderData {
    NEXT_ENVELOPE,
    SEND_TIME
};
typedef enum EnvelopeHeaderData EnvelopeHeaderData;

void k_initMessages(MemInfo *memInfo, MessageInfo *messageInfo);

int8_t k_sendMessage(MemInfo *memInfo, ProcInfo *procInfo, ProcId pid, Envelope *envelope);

Envelope *k_receiveMessage(MemInfo *memInfo, ProcInfo *procInfo, ProcId *senderPid);

int8_t k_delayedSend(MemInfo *memInfo, MessageInfo *messageInfo, ProcInfo *procInfo, ProcId pid, Envelope *envelope, uint32_t delay);

#endif
