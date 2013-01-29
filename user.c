
#include "user.h"
#include "proc.h"

void nullProcess() {
  while (1) {
    releaseProcessor();
  }
}

void funProcess() {
  int i;
  for (i = 0; i < 100; ++i) {
		//uart0_put_string("Hi\n\r");
    releaseProcessor();
  }
}

