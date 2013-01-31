#ifndef PROC_H
#define PROC_H

#include <stdint.h>

#define NUM_PROCS 4

// ProcId is used to store pids and is typedef'd
// to distinguish it from regular integers.
typedef uint8_t ProcId;

#define PROC_ID_KERNEL    (0x80)
#define PROC_ID_ALLOCATOR (0x81)
#define PROC_ID_NONE      (0xff)

enum ProcState {
    READY,
    RUNNING,
};
typedef enum ProcState ProcState;


typedef struct PCB PCB;
struct PCB {
  uint32_t *stack;
  ProcId pid;
  ProcState state;

  uint32_t priority;
};

typedef struct ProcInfo ProcInfo;
struct ProcInfo {
  PRQ prq; // Process ready queue
  PCB processes[NUM_PROCS]; // Actual process blocks
  PRQEntry *procQueue[NUM_PROCS];
  PCB *currentProcess;
};

/**
 * Returns PID
 */
void k_initProcesses(void);
uint32_t k_releaseProcessor(void);

#endif

