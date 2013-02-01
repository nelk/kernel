#include <LPC17xx.h>
#include "mem.h"
#include "proc.h"
#include "uart_polling.h"
#include "rtx.h"

extern uint32_t Image$$RW_IRAM1$$ZI$$Limit;
MemInfo gMem;
ProcInfo procInfo;

void k_memInitGlobal(void);

int main () {
	SystemInit();
	__disable_irq();
	uart1_init();
	uart1_put_string("Jizzilation");
	k_memInitGlobal();
  k_initProcesses(&procInfo);
  __enable_irq();

  // Transition to unprivileged level; default MSP is used
  __set_CONTROL(__get_CONTROL() | BIT(0));
	
  // k_releaseProcessor(&procInfo, YIELD); FUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU
	release_processor();
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
