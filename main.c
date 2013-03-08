#include <LPC17xx.h>
#include "common.h"
#include "mem.h"
#include "proc.h"
#include "rtx.h"
#include "uart_polling.h"

extern uint32_t Image$$RW_IRAM1$$ZI$$Limit;
MemInfo gMem;
MessageInfo messageInfo;
ProcInfo procInfo;

void k_memInitGlobal(void);

int main () {
    SystemInit();
    __disable_irq();
    uart_init(UART_NUM);
    uart_put_string(UART_NUM, "Starting up!\r\n");

    k_memInitGlobal();
    k_initProcesses(&gMem, &procInfo);
    k_initMessages(&gMem, &messageInfo);
    __enable_irq();

    // Transition to unprivileged level and release processor; default MSP is used
    __set_CONTROL(__get_CONTROL() | BIT(0));
    release_processor();
    return 0;
}

// Initialize global variables.
void k_memInitGlobal(void) {
    uint32_t memStartAddr = (uint32_t)&Image$$RW_IRAM1$$ZI$$Limit;
    k_memInfoInit(
            &gMem,
            memStartAddr,           // startAddr
            0x10008000,             // endAddr
            BLOCKSIZE_BYTES,        // blockSizeBytes = 128 bytes
            1                       // trackOwners = true
            );
}
