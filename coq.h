#ifndef COQ_H
#define COQ_H

#include "kernel_types.h"

// This file defines the methods for the CrtOutputQueue type
uint8_t hasData(CrtOutputQueue* coq, MemInfo *memInfo);
uint8_t getData(CrtOutputQueue* coq, MemInfo *memInfo);
void pushEnvelope(CrtOutputQueue* coq, Envelope* env);

#endif
