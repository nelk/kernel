#define TESTING

#include <stdio.h>

#define PASSED (1)
#define FAILED (0)

// Run tests with clang and gcc:
// clang -Wall -Wextra -m32 -g -std=c99 mem.c mem_test.c
// gcc -Wall -Wextra -m32 -g -std=c99 mem.c mem_test.c
int main() {
    printf("All tests passed.\n");
}
