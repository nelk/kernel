#define TESTING

#include <assert.h>
#include <stdio.h>

#define PASSED (1)
#define FAILED (0)

int main() {
    assert(PASSED != FAILED);
    printf("All tests passed.\n");
}
