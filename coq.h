#ifndef COQ_H
#define COQ_H

#include "kernel_types.h"

// This file defines the methods for the CrtOutputQueue type
uint8_t hasData(CrtOutputQueue* coq);
uint8_t getData(CrtOutputQueue* coq);
void pushEnvelope(CrtOutputQueue* coq, Envelope* env);

#endif
