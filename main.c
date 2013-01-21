#include <LPC17xx.h>
#include "mem.h"
#include "proc.h"
#include "uart_polling.h"

int main () {
	SystemInit();
	uart0_init();
  /*uart0_put_string("Hello World!\n\r");*/
	k_memInit();
  k_initProcesses();

  /* transit to unprivileged level, default MSP is used */
  __set_CONTROL(__get_CONTROL() | BIT(0));
  runProcessor();
	return 0;
}
