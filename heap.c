#include "heap.h"

void heapZero(heap *h) {
  h->lessFn = NULL;
  h->swapFn = NULL;
  h->ctx = NULL;
  h->len = 0;
}

void heapSetLessFn(heap *h, heapLessFunc fn) {
  h->lessFn = fn;
}

void heapSetSwapFn(heap *h, heapSwapFunc fn) {
  h->swapFn = fn;
}

void heapSetContext(heap *h, void *ctx) {
  h->ctx = ctx;
}

void heapBubbleUp(heap *h, size_t cur);
void heapBubbleDown(heap *h, size_t cur);

void heapInit(heap *h, size_t n) {
  size_t iter = 0;
  h->len = n;
  for (iter = n/2; iter > 0; iter--) {
    heapBubbleDown(h, iter-1);
  }
}

void heapAdd(heap* h) {
  ++(h->len);
  heapBubbleUp(h, h->len - 1);
}

void heapRemove(heap *h, size_t i) {
  size_t lastIndex = 0;
  if (i >= h->len) {
    return;
  }

  // NOTE: cannot do this before oob-check, or else we risk underflow.
  lastIndex = h->len - 1;

  if (lastIndex != i) {
    h->swapFn(h->ctx, lastIndex, i);
  }
  --(h->len);

  heapBubbleDown(h, i);
  heapBubbleUp(h, i);
}

void heapInvalidate(heap *h, size_t i) {
  if (i >= h->len) {
    return;
  }

  heapBubbleDown(h, i);
  heapBubbleUp(h, i);
}

void heapBubbleUp(heap *h, size_t cur) {
  size_t parent = 0;
  while (1) {
    // If we're at the root, then we're done.
    if (cur == 0) {
        break;
    }

    // If we are greater than or equal to our parent, then we're done.
    parent = (cur - 1) / 2;
    if (!h->lessFn(h->ctx, cur, parent)) {
      break;
    }

    // Otherwise, swap with our parent.
    h->swapFn(h->ctx, cur, parent);
    cur = parent;
  }
}

void heapBubbleDown(heap *h, size_t cur) {
  size_t leftChild = 0;
  size_t rightChild = 0;
  size_t leastChild = 0;
  size_t n = h->len;

  while (1) {
    leftChild = (2*cur) + 1;
    rightChild = leftChild + 1;

    // If we have no children, then we are done.
    if (leftChild >= n) {
      break;
    }

    // We speculatively assume that our least child is our left one.
    leastChild = leftChild;

    // If the right child is in range, and the left child is greater than or
    // equal to the right child, then we set our least child to our right child.
    if (rightChild < n && !h->lessFn(h->ctx, leftChild, rightChild)) {
      leastChild = rightChild;
    }

    // If our leastChild is greater than or equal to us, then we're done.
    if (!h->lessFn(h->ctx, leastChild, cur)) {
      break;
    }

    // Otherwise, swap and continue down.
    h->swapFn(h->ctx, cur, leastChild);
    cur = leastChild;
  }
}
