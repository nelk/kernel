#ifndef CRT_H
#define CRT_H

#include "kernel_types.h"

void crt_init(CRTData *);

uint8_t crt_hasOutByte(CRTData *);
uint8_t crt_getOutByte(CRTData *); // out_proc dequeues here

void crt_pushProcEnv(CRTData *, Envelope *env); // out_proc enqueues here
void crt_pushUserByte(CRTData *, uint8_t c); // in_proc enqueues here

uint8_t crt_hasFreeEnv(CRTData *);
Envelope *crt_getFreeEnv(CRTData *);


#endif
