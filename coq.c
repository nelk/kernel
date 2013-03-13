#include "coq.h"
#include "mem.h"

void advance(CrtOutputQueue *coq, MemInfo *memInfo) {
    if (coq->advanced) {
        return;
    }

    while (
        coq->head != NULL &&
        (
            coq->readIndex >= MESSAGEDATA_SIZE_BYTES ||
            coq->head->messageData[coq->readIndex] == '\0'
        )
    ) {
        Envelope *temp = coq->head;
        coq->head = coq->head->next;
        coq->readIndex = 0;
        if (coq->head == NULL) {
            coq->tail = NULL;
        }
        coq->readIndex = 0;
				rlsMemBlock(memInfo, (uint32_t)temp, CRT_PID , NULL);
    }

    coq->advanced = 1;
}

uint8_t hasData(CrtOutputQueue* coq, MemInfo *memInfo) {
    advance(coq, memInfo);
    return coq->head != NULL;
}

uint8_t getData(CrtOutputQueue* coq, MemInfo *memInfo) {
    advance(coq, memInfo);
    if (!hasData(coq, memInfo)) {
        return 0;
    }
    coq->advanced = 0;
    return coq->head->messageData[(coq->readIndex)++];
}

void pushEnvelope(CrtOutputQueue* coq, Envelope* env) {
    coq->advanced = 0;
    env->next = NULL;
    if (coq->tail == NULL) {
        coq->head = env;
        coq->tail = env;
    } else {
        coq->tail->next = env;
        coq->tail = env;
    }
}
