#ifndef TIMER_H
#define TIMER_H

#include "types.h"

uint32_t k_getTime(ClockInfo *clockInfo);
void k_initClock(ClockInfo *clockInfo);
void k_timerIRQHandler(ClockInfo *clockInfo);

#endif
