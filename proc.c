
#include <stdint.h>
#include "LPC17xx.h"
#include "proc.h"

PCB processes[PROC_HEAP_SIZE]; // Actual process blocks
PCB *procHeap[PROC_HEAP_SIZE]; // Pointers to PCBs
PCB *currentProcess = NULL;
uint32_t heapSize = 0;

/**
 * Min Heap implementation for PCBs
 */
PCB* heapTop() {
  return heap[0];
}

int heapify(PCB* heap[]) {
  heapify(heap, 0);
}

int heapify(PCB* heap[], int i) {
  PCB *temp;
  // If higher priority
  if (heap[i].priority < heap[(i-1)/2].priority) {
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
  heapify(heap);
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
    if (i*2 + childIndex < heapSize && heap[i].priority > heap[i*2 + childIndex].priority) {
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
    releaseProcessor();
  }
}

void funProcess() {
  int i;
  for (i = 0; i < 100; ++i) {
	  uart0_put_string("Hi\n\r");
    releaseProcessor();
  }

  // Who knows what happens next?
}

void k_initProcesses() {
  uint32_t i;
  for (i = 0; i < PROC_HEAP_SIZE; ++i) {
    processes[i].pid = i;
    processes[i].procState = NEW;
    processes[i].stackPointer = 0x0;
  }

  processes[0].stackPointer = (uint32_t) nullProcess;
  processes[0].priority = 6;
  processes[0].state = NEW;

  currentProcess = heap + i;
}

void switchToProcess(void *p) {
  __set_MSP((uint32_t) p);
  __rte(); // Not sure exactly what this does.
}


void runProcessor() {
  while (1) {
    // Save old process info
    currentProcess->state = READY;
		currentProcess->stackPointer = (uint32_t *) __get_MSP(); /* save the old process's sp */

    PCB *nextProc = heapTop(heap);
    nextProc->state = RUNNING;
    switchToProcess(nextProc->stackPointer);
  }
}

