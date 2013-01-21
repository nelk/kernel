
#ifndef PROC_H
#define PROC_H

#include <stdint.h>

#define PROC_HEAP_SIZE 6

typedef enum  ProcState ProcState;
enum ProcState {NEW, READY, RUNNING};

typedef struct PCB PCB;
struct PCB {
  uint32_t *stackPointer;
  uint32_t pid;
  ProcState procState;
  uint32_t priority;
};

/**
 * Returns PID
 */
//uint32_t k_createProcess();
//uint32_t k_releaseProcess();
void k_initProcesses();
void releaseProcessor();

void runProcessor();

#endif

