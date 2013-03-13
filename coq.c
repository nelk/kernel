#include "coq.h"

void advance(CrtOutputQueue *coq) {
    if (coq->advanced) {
        return;
    }

    coq->advanced = 1;
    while (
        coq->head != NULL &&
        (
            coq->readIndex >= MESSAGEDATA_SIZE_BYTES ||
            coq->head->messageData[readIndex] == '\0'
        )
    ) {
        Envelope *temp = coq->head;
        coq->head = head->next;
        coq->readIndex = 0;
        if (coq->head == NULL) {
            coq->tail = NULL;
        }
        coq->readIndex = 0;
        release_memory_block((void*)temp);
    }
}

uint8_t hasData(CrtOutputQueue* coq) {
    advance(coq);
    return coq->head != NULL;
}

uint8_t getData(CrtOutputQueue* coq) {
    advance(coq);
    if (!hasData(coq)) {
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
