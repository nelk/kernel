
#include "user.h"
#include "proc.h"

void releaseProcessor();

void nullProcess(void) {
  while (1) {
    releaseProcessor();
  }
}

void funProcess(void) {
  int i;
  for (i = 0; i < 100; ++i) {
		//uart0_put_string("Hi\n\r");
    releaseProcessor();
  }
}

