#include <LPC17xx.h>
#include "mem.h"
#include "uart_polling.h"

int main () {
	SystemInit();
	uart0_init();
	uart0_put_string("Hello World!\n\r");
	k_memInit();
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
