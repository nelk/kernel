#include <LPC17xx.h>
#include "kernel_types.h"
#include "mem.h"
#include "message.h"
#include "proc.h"
#include "rtx.h"
#include "timer.h"
#include "uart_polling.h"
#include "type.h"
#include "dac.h"

extern uint32_t Image$$RW_IRAM1$$ZI$$Limit;
ClockInfo gClockInfo;
MemInfo gMemInfo;
MessageInfo gMessageInfo;
ProcInfo gProcInfo;

void k_memInitGlobal(void);

int main () {
		uint32_t i = 0, m;
    SystemInit();
    __disable_irq();
    uart_init(UART_NUM);

    k_memInitGlobal();
    k_initProcesses(&gProcInfo, &gMemInfo);
    k_initMessages(&gMessageInfo, &gMemInfo);
    k_initClock(&gClockInfo);
    __enable_irq();
		DACInit();
	
		while ( 1 )
		{
					*(int *)(0x0C000 + 0x40080000UL) = (i << 6) | 0x00010000;
					i++;
					for(m = 1000; m > 1; m--);
					if ( i == 1024 )
					{
						i = 0;
					}
		}

    // Transition to unprivileged level and release processor; default MSP is used
//    __set_CONTROL(__get_CONTROL() | BIT(0));
//    release_processor();
    return 0;
}

// Initialize global variables.
void k_memInitGlobal(void) {
    uint32_t memStartAddr = (uint32_t)&Image$$RW_IRAM1$$ZI$$Limit;
    k_memInfoInit(
            &gMemInfo,
            memStartAddr,           // startAddr
            0x10008000,             // endAddr
            BLOCKSIZE_BYTES,        // blockSizeBytes = 128 bytes
            1                       // trackOwners = true
            );
}
