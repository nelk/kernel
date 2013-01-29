
#ifndef PROC_H
#define PROC_H

#include <stdint.h>

#define NUM_PROCS 6

enum ProcState {NEW, READY, RUNNING};
typedef enum ProcState ProcState;

typedef struct PCB PCB;
struct PCB {
  uint32_t *stack;
  uint32_t pid;
  ProcState state;

  uint32_t priority;
};

/**
 * Returns PID
 */
void k_initProcesses(void);
uint32_t k_releaseProcessor(void);

#endif

