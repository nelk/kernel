#include <LPC17xx.h>
#include "mem.h"
#include "proc.h"
#include "uart_polling.h"

extern uint32_t Image$$RW_IRAM1$$ZI$$Limit;
MemInfo gMem;
ProcInfo procInfo;

void k_memInitGlobal(void);

int main () {
	SystemInit();
	uart0_init();
	k_memInitGlobal();
  k_initProcesses(&procInfo);

  // Transition to unprivileged level; default MSP is used
  __set_CONTROL(__get_CONTROL() | BIT(0));
  k_releaseProcessor(&procInfo, YIELD);
	return 0;
}

// Initialize global variables.
void k_memInitGlobal(void) {
    uint32_t memStartAddr = (uint32_t)&Image$$RW_IRAM1$$ZI$$Limit;
    k_memInfoInit(
        &gMem,
        memStartAddr,  // startAddr
        0x10008000,    // endAddr
        1 << 7         // blockSizeBytes = 128 bytes
    );
}
