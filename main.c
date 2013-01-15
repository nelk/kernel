#include <LPC17xx.h>
#include "uart_polling.h"
#include "mem.h"

int main () {
	SystemInit();
	uart0_init();
	uart0_put_string("Hello World!\n\r");
	k_memInit();
	return 0;
}
