
#include "proc.h"
#include "rtx.h"
#include "user.h"

// "User Land"

void nullProcess(void) {
  while (1) {
    release_processor();
  }
}

void funProcess(void) {
  int i;
  for (i = 0; i < 100; ++i) {
		//uart0_put_string("Hi\n\r");
    release_processor();
  }
}

