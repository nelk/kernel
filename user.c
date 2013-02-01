
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

void itoa(char *buf, uint8_t i) {
	*(buf++) = (i / 100) + '0';
	i %= 100;
	*(buf++) = (i / 10) + '0';
	i %= 10;
	*(buf++) = i + '0';
}

void fibProcess(void) {
	uint8_t temp;
	uint8_t cur;
	uint8_t prev;
	uint8_t idx;
	char *str = "fib(000) = 000";
	uint8_t *buf = (uint8_t *)request_memory_block();
	uint8_t *iterl = NULL;
	uint8_t *iterr = NULL;

	iterl = str;
	iterr = buf;
	while (*iterl != '\0') {
		*(iterr++) = *(iterl++);
	}
	*iterr = '\0';

	while (1) {
		prev = 1;
		cur = 1;
		idx = 0;
		while (cur < 200) {
			temp = prev;
			prev = cur;
			cur = cur + temp;
			idx++;

			itoa(buf + 4, idx);
			itoa(buf + 11, cur);

			uart1_put_string(buf);

			if (idx % 5 == 0) {
				release_processor();
			}
		}

		release_processor();
	}
}
