#define TESTING

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "mem.h"

#define PASSED (1)
#define FAILED (0)

uint32_t k_getAlignedStartAddress(uint32_t, uint32_t);
uint32_t k_getAlignedEndAddress(uint32_t, uint32_t);
int testAlignedStartAddress() {
    typedef struct {
        uint32_t blockSize;

        uint32_t start;
        uint32_t end;

        uint32_t expectedStart;
        uint32_t expectedEnd;
    } testcase;

    testcase cases[] = {
        {2, 0, 6, 0, 6},
        {2, 1, 6, 2, 6},
        {2, 0, 5, 0, 4},
        {2, 1, 5, 2, 4},
    };

    int len = sizeof(cases)/sizeof(cases[0]);

    int result = PASSED;

    for (int i = 0; i < len; i++) {
        uint32_t gotStart = k_getAlignedStartAddress(
            cases[i].start,
            cases[i].blockSize
        );

        uint32_t gotEnd = k_getAlignedEndAddress(
            cases[i].end,
            cases[i].blockSize
        );

        if (
            gotStart == cases[i].expectedStart &&
            gotEnd == cases[i].expectedEnd
        ) {
            continue;
        }

        result = FAILED;

        if (gotStart != cases[i].expectedStart) {
            printf(
                "for k_getAlignedStartAddress(%d, %d), got %d, expected %d\n",
                cases[i].start,
                cases[i].blockSize,
                gotStart,
                cases[i].expectedStart
            );
        }
        if (gotEnd != cases[i].expectedEnd) {
            printf(
                "for k_getAlignedEndAddress(%d, %d), got %d, expected %d\n",
                cases[i].end,
                cases[i].blockSize,
                gotEnd,
                cases[i].expectedEnd
            );
        }
    }

    return result;
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

    MemInfo memInfo;

    for (int i = 0; i < len; i++) {
        k_memInfoInit(&memInfo, cases[i].startAddr, 0x10000000, cases[i].blockSizeBytes, 1);

        uint32_t got = (uint32_t)k_findOwnerSlot(&memInfo, cases[i].addr);

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

    return result;
}

int testMultipleArenas() {
    uint8_t *backingStorage = (uint8_t *)malloc(100000*sizeof(uint8_t));
    assert(backingStorage);
    MemInfo memInfo;

    k_memInfoInit(
        &memInfo,
        (uint32_t)backingStorage,           // startAddr
        ((uint32_t)backingStorage) + 65536, // endAddr
        4,                                  // blockSizeBytes
        1                                   // trackOwners
    );

    int firstPid = 1;
    uint32_t firstBlock = k_acquireMemoryBlock(&memInfo, firstPid);
    assert(firstBlock);

    // Check permissions
    assert(k_isOwnerUnsafe(&memInfo, firstBlock, firstPid));
    assert(k_isOwnerUnsafe(&memInfo, (uint32_t)backingStorage, PROC_ID_ALLOCATOR));

    // Eat up the entire first arena, i.e. 2 more blocks
    k_acquireMemoryBlock(&memInfo, firstPid);
    k_acquireMemoryBlock(&memInfo, firstPid);

    // The next acquire should be in the second arena
    int secondPid = 2;
    uint32_t secondBlock = k_acquireMemoryBlock(&memInfo, secondPid);
    assert(secondBlock);

    // Check permissions
    assert(k_isOwnerUnsafe(&memInfo, firstBlock, firstPid));
    assert(k_isOwnerUnsafe(&memInfo, memInfo.startMemoryAddress, PROC_ID_ALLOCATOR));
    assert(k_isOwnerUnsafe(&memInfo, secondBlock, secondPid));
    assert(k_isOwnerUnsafe(&memInfo, secondBlock+memInfo.blockSizeBytes, PROC_ID_NONE));
    assert(k_isOwnerUnsafe(&memInfo, memInfo.startMemoryAddress+16, PROC_ID_ALLOCATOR));

    free(backingStorage);
    return PASSED;
}

int testMemOperations() {
    uint8_t *backingStorage = (uint8_t *)malloc(100000*sizeof(uint8_t));
    assert(backingStorage);
    MemInfo memInfo;

    k_memInfoInit(
        (&memInfo),
        (uint32_t)backingStorage,           // startAddr
        ((uint32_t)backingStorage) + 65536, // endAddr
        16,                                 // blockSizeBytes
        1                                   // trackOwners
    );

    int firstPid = 1;
    uint32_t firstBlock = k_acquireMemoryBlock((&memInfo), firstPid);
    assert(firstBlock);

    // Check permissions
    assert(k_isOwnerUnsafe((&memInfo), firstBlock, firstPid));
    assert(k_isOwnerUnsafe((&memInfo), memInfo.startMemoryAddress, PROC_ID_ALLOCATOR));
    assert(k_isOwnerUnsafe((&memInfo), firstBlock+16, PROC_ID_NONE));

    int secondPid = 2;
    uint32_t secondBlock = k_acquireMemoryBlock((&memInfo), secondPid);
    assert(secondBlock);

    // Check permissions
    assert(k_isOwnerUnsafe((&memInfo), firstBlock, firstPid));
    assert(k_isOwnerUnsafe((&memInfo), memInfo.startMemoryAddress, PROC_ID_ALLOCATOR));
    assert(k_isOwnerUnsafe((&memInfo), secondBlock, secondPid));
    assert(k_isOwnerUnsafe((&memInfo), secondBlock+16, PROC_ID_NONE));

    // Manually add to free list, and verify that acquire takes from free list
    k_setOwnerUnsafe((&memInfo), secondBlock, PROC_ID_NONE);
    FreeBlock *fb = (FreeBlock *)secondBlock;
    (&memInfo)->firstFree = fb;

    int thirdPid = 3;
    uint32_t thirdBlock = k_acquireMemoryBlock((&memInfo), thirdPid);
    assert(thirdBlock == secondBlock);
    assert(k_isOwnerUnsafe((&memInfo), thirdBlock, thirdPid));
    assert((&memInfo)->firstFree == NULL);


    // Test happy path
    int ret = k_releaseMemoryBlock((&memInfo), thirdBlock, thirdPid);
    assert(ret == SUCCESS);
    assert(k_isOwnerUnsafe((&memInfo), thirdBlock, PROC_ID_NONE));
    assert((&memInfo)->firstFree != NULL);

    // Test double-free fails
    ret = k_releaseMemoryBlock((&memInfo), thirdBlock, thirdPid);
    assert(ret == EPERM);

    // Test wrong owner fails
    ret = k_releaseMemoryBlock((&memInfo), secondBlock, thirdPid);
    assert(ret == EPERM);

    // Test internal block pointer fails
    ret = k_releaseMemoryBlock((&memInfo), secondBlock + 1, secondPid);
    assert(ret == EINVAL);

    // Test before beginning of memory
    ret = k_releaseMemoryBlock(
        (&memInfo),
        (&memInfo)->startMemoryAddress - 1,
        PROC_ID_ALLOCATOR
    );
    assert(ret == EINVAL);

    // Test after end of memory
    ret = k_releaseMemoryBlock((&memInfo), (&memInfo)->endMemoryAddress, PROC_ID_KERNEL);
    assert(ret == EINVAL);

    // Test after nextAvailableAddress. We shouldn't allow
    // programs to release memory blocks after that point
    // because the header is not allocated yet, and might be
    // corrupted.
    uint32_t attackArenaAddr = (&memInfo)->startMemoryAddress + (&memInfo)->arenaSizeBytes;
    uint32_t attackAddr = attackArenaAddr + (&memInfo)->blockSizeBytes;
    k_setOwnerUnsafe((&memInfo), attackAddr, firstPid);
    ret = k_releaseMemoryBlock((&memInfo), attackAddr, firstPid);
    assert(ret == EINVAL);

    // Test OOM
    (&memInfo)->endMemoryAddress = 0;
    (&memInfo)->firstFree = NULL;
    int oomPid = 4;
    uint32_t oomBlock = k_acquireMemoryBlock((&memInfo), oomPid);
    assert(oomBlock == 0);

    free(backingStorage);
    return PASSED;
}

// TODO(sanjay): test k_validMemoryBlock, it is only tested implicitly
// by the release memory block tests.

int main() {
    assert(PASSED != FAILED);
    assert(testFindOwnerSlot() == PASSED);
    assert(testAlignedStartAddress() == PASSED);
    assert(testMemOperations() == PASSED);
    assert(testMultipleArenas() == PASSED);

    printf("All tests passed.\n");
}
