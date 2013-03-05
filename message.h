#ifndef MESSAGE_H
#define MESSAGE_H

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
    char messageData; // TODO(alex) - make this an array so the user won't be confused
};


int8_t k_sendMessage(MemInfo *memInfo, ProcInfo *procInfo, uint8_t pid, Envelope *envelope) {
    // TODO
}

int8_t k_receiveMessage(MemInfo *memInfo, ProcInfo *procInfo, uint8_t *senderPid) {
    // TODO
}

int8_t k_delayedSend(MemInfo *memInfo, ProcInfo *procInfo, uint8_t pid, Envelope *envelope, uint32_t delay) {
    // TODO
}

#endif
