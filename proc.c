
#include <stdint.h>
#include <stddef.h>
//#include "LPC17xx.h"
#include "proc.h"
#include "mem.h"

void __rte(void);
void  __set_MSP(uint32_t);
uint32_t __get_MSP(void);

PCB processes[PROC_HEAP_SIZE]; // Actual process blocks
PCB *procHeap[PROC_HEAP_SIZE]; // Pointers to PCBs
PCB *currentProcess = NULL;
uint32_t heapSize = 0;

/**
 * Min Heap implementation for PCBs
 */
PCB* heapTop(PCB* heap[]) {
  return heap[0];
}

void heapify(PCB* heap[], int i) {
  PCB *temp;
  //TODO make iterative
  if (heap[i]->priority < heap[(i-1)/2]->priority) {
    temp = heap[i];
    heap[i] = heap[(i-1)/2];
    heap[(i-1)/2] = temp;
    if (i > 0) {
      heapify(heap, (i-1)/2);
    }
  }
}

int heapAdd(PCB* heap[], PCB *pcb) {
  if (heapSize >= PROC_HEAP_SIZE) {
    return -1;
  }
  heap[heapSize] = pcb;
  ++heapSize;
  heapify(heap, 0);
  return 0;
}

void heapRemove(PCB* heap[], int i) {
  int childIndex;
  PCB *temp;

  temp  = heap[i];
  heap[i] = heap[heapSize - 1];
  heap[heapSize - 1] = temp;
  --heapSize;
  heapify(heap, i);

  // Heapify down
  for (childIndex = 1; childIndex < 3; ++childIndex) {
    if (i*2 + childIndex < heapSize && heap[i]->priority > heap[i*2 + childIndex]->priority) {
      temp = heap[i];
      heap[i] = heap[i*2 + childIndex];
      heap[i*2 + childIndex] = temp;
    }
  }
}

/**
 * End Heap
 */

void nullProcess() {
  while (1) {
    k_releaseProcessor();
  }
}

void funProcess() {
  int i;
  for (i = 0; i < 100; ++i) {
		//uart0_put_string("Hi\n\r");
    k_releaseProcessor();
  }
}

void k_initProcesses(void) {
  PCB *process;
  uint32_t i;
  for (i = 0; i < PROC_HEAP_SIZE; ++i) {
    process = &processes[i];
    process->pid = i;
    process->state = NEW;
    //TODO - assert that these memory blocks are contigious
    k_acquireMemoryBlock(i);
    process->stack = (uint32_t *)((uint32_t)k_acquireMemoryBlock(i) + gMem.blockSizeBytes);
  }

  // Push process function address onto stack
  process = &processes[0];
  process->stack--;
  *(process->stack) = (uint32_t) nullProcess;
  process->priority = 6;
  process->state = RUNNING;
  heapAdd(procHeap, process);

  currentProcess = NULL;
}

int k_releaseProcessor(void) {
	PCB *nextProc;
  if (currentProcess != NULL) {
    // Save old process info
    currentProcess->state = READY;
    currentProcess->stack = (uint32_t *) __get_MSP(); /* save the old process's sp */
    heapAdd(procHeap, currentProcess);
  }

  nextProc = heapTop(procHeap);
  nextProc->state = RUNNING;
  heapRemove(procHeap, 0);
  currentProcess = nextProc;
  __set_MSP((uint32_t) nextProc->stack);
  __rte();

  return 0; // Not reachable
}


