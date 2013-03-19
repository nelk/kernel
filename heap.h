#ifndef HEAP_H
#define HEAP_H

#include <stddef.h>
#include <stdint.h>

// This header will define a "generic" heap that operates via
// a series of user-controlled callbacks. It is based on the
// Go programming language's open-source implementation of
// the heap data structure.
// See http://tip.golang.org/pkg/container/heap/ for docs,
// and http://tip.golang.org/src/pkg/container/heap/ for source.

typedef uint8_t (*heapLessFunc)(void *, size_t, size_t);
typedef void (*heapSwapFunc)(void *, size_t, size_t);

typedef struct heap heap;
struct heap {
    heapLessFunc lessFn;
    heapSwapFunc swapFn;
    void *ctx;
    size_t len;
};

// heapZero zeroes all internal fields of h, and should be called before
// using h. It runs in constant time.
void heapZero(heap *h);

// heapSetLessFn sets the internal comparator for h. It runs in constant time.
void heapSetLessFn(heap *h, heapLessFunc fn);

// heapSetSwapFn sets the internal swapper for h. It runs in constant time.
void heapSetSwapFn(heap *h, heapSwapFunc fn);

// heapSetContext sets the internal context for h. It runs in constant time.
// The context ctx will be passed to all callbacks without modification.
void heapSetContext(heap *h, void *ctx);

// heapInit initializes the heap h, setting its internal length to n.
// It runs in time linear in n. It is safe to call heapInit on
// a heap that has already been initialized. It assumes that h's swapper and
// comparator are already set.
void heapInit(heap *h, size_t n);

// heapAdd increases h's internal length by one, and rebalances the heap.
// It runs in time logarithmic in the length of the heap. It assumes that h's
// swapper and comparator are already set.
void heapAdd(heap *h);

// heapRemove removes the element at index i from heap h, and rebalances
// the heap. It runs in time logarithmic in the length of the heap. It assumes
// that h's swapper and comparator are already set. If i is greater than or
// equal to the heap's internal length, this is a no-op.
void heapRemove(heap *h, size_t i);

// heapInvalidate assumes that the element at index i from heap h is in the
// wrong position and performs the necessary actions to rebalance the heap
// to maintain heap invariants.
// It runs in time logarithmic in the length of the heap. It assumes that
// h's swapper and comparator are already set. If i is greater than or
// equal to the heap's internal length, this is a no-op.
// This function should be used when the value for a particular element
// has been updated, and its position in the heap might have been invalidated.
void heapInvalidate(heap *h, size_t i);

#endif
