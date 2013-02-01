
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
			uart1_put_string("Fun ");
			uart1_put_char(i + '0');
			uart1_put_string("\n\r");
		}
		release_processor();
	}
}

void schizophrenicProcess(void) {
	int i;
	while (1) {
		for (i = 9; i >= 5; --i) {
			uart1_put_string("Schizophrenic ");
			uart1_put_char(i + '0');
			uart1_put_string("\n\r");
		}
		release_processor();
	}
}

void memMunchProcess(void) {}
