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
