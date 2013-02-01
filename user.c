#include <stddef.h>

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
			uart_put_string(UART_NUM, "Fun ");
			uart_put_char(UART_NUM, i + '0');
			uart_put_string(UART_NUM, "\n\r");
		}
		release_processor();
	}
}

void schizophrenicProcess(void) {
	int i;
	while (1) {
		for (i = 9; i >= 5; --i) {
			uart_put_string(UART_NUM, "Schizophrenic ");
			uart_put_char(UART_NUM, i + '0');
			uart_put_string(UART_NUM, "\n\r");
		}
		release_processor();
	}
}

void print_uint32(uint32_t i) {
	int base = 1;
	while ((i / (base*10)) != 0) {
		base *= 10;
	}

	do {
		uart_put_char(UART_NUM, (i/base) + '0');
		i %= base;
		base /= 10;
	} while (i > 0);
}

void fibProcess(void) {
	uint8_t temp;
	uint8_t cur;
	uint8_t prev;
	uint8_t idx;

	while (1) {
		prev = 1;
		cur = 1;
		idx = 0;
		while (cur < 200) {
			temp = prev;
			prev = cur;
			cur = cur + temp;
			idx++;

			uart_put_string(UART_NUM, "fib(");
			print_uint32(idx);
			uart_put_string(UART_NUM, ") = ");
			print_uint32(cur);
			uart_put_string("\r\n");

			if (idx % 5 == 0) {
				release_processor();
			}
		}

		release_processor();
	}
}
