
#include "proc.h"
#include "rtx.h"
#include "uart_polling.h"
#include "user.h"

// User Land

void nullProcess(void) {
  while (1) {
    release_processor();
  }
}

void funProcess(void) {
  int i;
	while (1) {
		for (i = 0; i < 5; ++i) {
			uart0_put_string("Fun ");
			uart0_put_char(i + '0');
			uart0_put_string("\n\r");
		}
		release_processor();
	}
}

void schizophrenicProcess(void) {
	int i;
	while (1) {
		for (i = 9; i >= 5; --i) {
			uart0_put_string("Schizophrenic ");
			uart0_put_char(i + '0');
			uart0_put_string("\n\r");
		}
		release_processor();
	}
}

void memMunchProcess(void) {}
