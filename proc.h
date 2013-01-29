
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
//uint32_t k_createProcess();
//uint32_t k_releaseProcess();
void k_initProcesses();
int releaseProcessor();

void runProcessor();

#endif

