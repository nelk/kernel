#include <LPC17xx.h>
#include "mem.h"
#include "proc.h"
#include "uart_polling.h"

int main () {
	SystemInit();
	uart0_init();
	k_memInit();
  k_initProcesses();

  // Transition to unprivileged level; default MSP is used
  __set_CONTROL(__get_CONTROL() | BIT(0));
  k_releaseProcessor();
	return 0;
}

// Initialize global variables.
void k_memInit(MemInfo *memInfo) {
    uint32_t memStartAddr = (uint32_t)&Image$$RW_IRAM1$$ZI$$Limit;
    k_setInfo(
        memInfo,
        memStartAddr,  // startAddr
        0x10008000,    // endAddr
        1 << 7         // blockSizeBytes = 128 bytes
    );
}
