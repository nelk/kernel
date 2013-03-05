#ifndef MESSAGE_H
#define MESSAGE_H

#include "common.h"
#include "message.h"
#include "mem.h"
#include "proc.h"

// TODO(alex) - move Envelope to user-facing header (user_message.h) or aggregate user-facing types header.
typedef struct Envelope Envelope;
struct Envelope {
    uint32_t header[3];
    uint32_t senderPid;
    uint32_t destPid;
    uint32_t messageType;
    char messageData[BLOCKSIZE_BYTES - 6*sizeof(uint32_t)];
};

enum EnvelopeHeaderData {
    NEXT_ENVELOPE
};
typedef enum EnvelopeHeaderData EnvelopeHeaderData;

int8_t k_sendMessage(MemInfo *memInfo, ProcInfo *procInfo, ProcId pid, Envelope *envelope);

Envelope *k_receiveMessage(MemInfo *memInfo, ProcInfo *procInfo, ProcId *senderPid);

int8_t k_delayedSend(MemInfo *memInfo, ProcInfo *procInfo, ProcId pid, Envelope *envelope, uint32_t delay);

#endif
