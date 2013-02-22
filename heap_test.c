#define TESTING

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "heap.h"

#define PASSED (1)
#define FAILED (0)

typedef struct array array;
struct array {
  uint32_t *store;
  size_t len;
};

uint8_t arrayLess(void *ctx, size_t i, size_t j) {
  array *context = (array *)ctx;
  assert(i < context->len);
  assert(j < context->len);
  return context->store[i] < context->store[j];
}

uint8_t arrayGE(void *ctx, size_t i, size_t j) {
  return arrayLess(ctx, j, i);
}

void arraySwap(void *ctx, size_t i, size_t j) {
  array *context = (array *)ctx;
  assert(i < context->len);
  assert(j < context->len);
  uint32_t temp = context->store[i];
  context->store[i] = context->store[j];
  context->store[j] = temp;
}

int testHeapSort() {
  uint32_t store[10] = {0};
  uint32_t store2[10] = {0};
  array a;
  a.store = store;
  a.len = 10;

  heap h;
  heapZero(&h);
  heapSetLessFn(&h, &arrayGE);
  heapSetSwapFn(&h, &arraySwap);
  heapSetContext(&h, &a);

  srand(0);
  for (int cases = 0; cases < 10000; cases++) {
    for (size_t i = 0; i < a.len; i++) {
      store2[i] = rand();
      a.store[i] = store2[i];
    }

    // First, test using heapInit to do heapsort.
    heapInit(&h, a.len);
    for (size_t i = 0; i < a.len; i++) {
        heapRemove(&h, 0);
    }
    for (size_t i = 0; i < a.len - 1; i++) {
        assert(a.store[i] <= a.store[i+1]);
    }

    // Now, test using manual add, then remove.
    for (size_t i = 0; i < a.len; i++) {
      a.store[i] = store2[i];
      heapAdd(&h);
    }
    for (size_t i = 0; i < a.len; i++) {
        heapRemove(&h, 0);
    }
    for (size_t i = 0; i < a.len - 1; i++) {
        assert(a.store[i] <= a.store[i+1]);
    }
  }

  return PASSED;
}

int main() {
  assert(PASSED != FAILED);
  assert(testHeapSort() == PASSED);
  printf("All tests passed\n");
  return 0;
}
