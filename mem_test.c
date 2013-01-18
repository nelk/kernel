#define TESTING

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "mem.h"

#define PASSED (1)
#define FAILED (0)

uint32_t k_getAlignedStartAddress(uint32_t, uint32_t);
int testAlignedStartAddress() {
    typedef struct {
        uint32_t start;
        uint32_t blockSize;
        uint32_t expectedResult;
    } testcase;

    testcase cases[] = {
        {0, 2, 0},
        {1, 2, 2},
    };

    int len = sizeof(cases)/sizeof(cases[0]);

    int result = PASSED;

    for (int i = 0; i < len; i++) {
        uint32_t got = k_getAlignedStartAddress(
            cases[i].start,
            cases[i].blockSize
        );

        if (got == cases[i].expectedResult) {
            continue;
        }

        result = FAILED;
        printf(
            "for k_getAlignedStartAddress(%d, %d), got %d, expected %d\n",
            cases[i].start,
            cases[i].blockSize,
            got,
            cases[i].expectedResult
        );
    }

    return PASSED;
}

int testFindOwnerSlot() {
    typedef struct {
        uint32_t startAddr;
        uint32_t addr;
        uint32_t blockSizeBytes;

        uint32_t expectedResult;
    } testcase;

    testcase cases[] = {
        {0, 0, 4, 0},
        {0, 4, 4, 1},
        {0, 8, 4, 2},
        {0, 12, 4, 3},

        {16, 16, 4, 16},
        {16, 20, 4, 17},
        {16, 24, 4, 18},
        {16, 28, 4, 19},

        {0, 0, 2, 0},
        {0, 2, 2, 1},

        {4, 4, 2, 4},
        {4, 6, 2, 5},

        {16, 48, 4, 48},
        {16, 52, 4, 49},
        {16, 56, 4, 50},
        {16, 60, 4, 51},

        {4, 8, 2, 8},
        {4, 10, 2, 9},
    };

    int len = sizeof(cases)/sizeof(cases[0]);

    int result = PASSED;

    for (int i = 0; i < len; i++) {
        k_setGlobals(cases[i].startAddr, 0x10000000, cases[i].blockSizeBytes);

        uint32_t got = (uint32_t)k_findOwnerSlot(cases[i].addr);

        if (got == cases[i].expectedResult) {
            continue;
        }

        result = FAILED;
        printf(
            "got %d, expected %d\n",
            got,
            cases[i].expectedResult
        );
    }

    return PASSED;
}

int testMultipleArenas() {
    uint8_t *backingStorage = (uint8_t *)malloc(100000*sizeof(uint8_t));
    assert(backingStorage);

    k_setGlobals(
        (uint32_t)backingStorage,           // startAddr
        ((uint32_t)backingStorage) + 65536, // endAddr
        2                                   // blockSizeBytes
    );

    int firstPid = 1;
    void *firstBlock = k_acquireMemoryBlock(firstPid);
    assert(firstBlock);

    // Check permissions
    assert(k_getOwner((uint32_t)firstBlock) == firstPid);
    assert(k_getOwner((uint32_t)backingStorage) == PROC_ID_ALLOCATOR);

    int secondPid = 2;
    void *secondBlock = k_acquireMemoryBlock(secondPid);
    assert(secondBlock);

    // Check permissions
    assert(k_getOwner((uint32_t)firstBlock) == firstPid);
    assert(k_getOwner((uint32_t)backingStorage) == PROC_ID_ALLOCATOR);
    assert(k_getOwner((uint32_t)secondBlock) == secondPid);
    assert(k_getOwner((uint32_t)secondBlock+2) == PROC_ID_NONE);
    assert(k_getOwner((uint32_t)backingStorage+4) == PROC_ID_ALLOCATOR);

    return PASSED;
}

int testMemOperations() {
    uint8_t *backingStorage = (uint8_t *)malloc(100000*sizeof(uint8_t));
    assert(backingStorage);

    k_setGlobals(
        (uint32_t)backingStorage,           // startAddr
        ((uint32_t)backingStorage) + 65536, // endAddr
        16                                  // blockSizeBytes
    );

    int firstPid = 1;
    void *firstBlock = k_acquireMemoryBlock(firstPid);
    assert(firstBlock);

    // Check permissions
    assert(k_getOwner((uint32_t)firstBlock) == firstPid);
    assert(k_getOwner((uint32_t)backingStorage) == PROC_ID_ALLOCATOR);
    assert(k_getOwner((uint32_t)firstBlock+16) == PROC_ID_NONE);

    int secondPid = 2;
    void *secondBlock = k_acquireMemoryBlock(secondPid);
    assert(secondBlock);

    // Check permissions
    assert(k_getOwner((uint32_t)firstBlock) == firstPid);
    assert(k_getOwner((uint32_t)backingStorage) == PROC_ID_ALLOCATOR);
    assert(k_getOwner((uint32_t)secondBlock) == secondPid);
    assert(k_getOwner((uint32_t)secondBlock+16) == PROC_ID_NONE);

    // Manually add to free list, and verify that acquire takes from free list
    k_setOwner((uint32_t)secondBlock, PROC_ID_NONE);
    FreeBlock *fb = (FreeBlock *)secondBlock;
    gMem.firstFree = fb;

    int thirdPid = 3;
    void *thirdBlock = k_acquireMemoryBlock(thirdPid);
    assert(thirdBlock == secondBlock);
    assert(k_getOwner((uint32_t)thirdBlock) == thirdPid);
    assert(gMem.firstFree == NULL);


    // Test happy path
    int ret = k_releaseMemoryBlock(thirdBlock, thirdPid);
    assert(ret == 0);
    assert(k_getOwner((uint32_t)thirdBlock) == PROC_ID_NONE);
    assert(gMem.firstFree != NULL);

    // Test double-free fails
    ret = k_releaseMemoryBlock(thirdBlock, thirdPid);
    assert(ret != 0);

    // Test wrong owner fails
    ret = k_releaseMemoryBlock(secondBlock, thirdPid);
    assert(ret != 0);

    // Test internal block pointer fails
    ret = k_releaseMemoryBlock(secondBlock + 1, secondPid);
    assert(ret != 0);

    // Test before beginning of memory
    ret = k_releaseMemoryBlock(
        (void *)(gMem.startMemoryAddress - 1),
        PROC_ID_ALLOCATOR
    );
    assert(ret != 0);

    // Test after end of memory
    ret = k_releaseMemoryBlock((void *)gMem.endMemoryAddress, PROC_ID_KERNEL);
    assert(ret != 0);

    // Test after nextAvailableAddress. We shouldn't allow
    // programs to release memory blocks after that point
    // because the header is not allocated yet, and might be
    // corrupted.
    uint32_t attackArenaAddr = gMem.startMemoryAddress + gMem.arenaSizeBytes;
    uint32_t attackAddr = attackArenaAddr + gMem.blockSizeBytes;
    k_setOwner(attackAddr, firstPid);
    ret = k_releaseMemoryBlock((void *)attackAddr, firstPid);
    assert(ret != 0);

    // Test OOM
    gMem.endMemoryAddress = 0;
    gMem.firstFree = NULL;
    int oomPid = 4;
    void *oomBlock = k_acquireMemoryBlock(oomPid);
    assert(oomBlock == NULL);

    free(backingStorage);
    return PASSED;
}

uint32_t Image$$RW_IRAM1$$ZI$$Limit;

// Run tests with clang and gcc:
// clang -Wall -Wextra -m32 -g -std=c99 mem.c mem_test.c
// gcc -Wall -Wextra -m32 -g -std=c99 mem.c mem_test.c
int main() {
    assert(testAlignedStartAddress() == PASSED);
    assert(testMemOperations() == PASSED);
    assert(testMultipleArenas() == PASSED);
    assert(testFindOwnerSlot() == PASSED);
}
