#include <stddef.h>

#include "mem.h"

MemInfo gMem;

// This function assumes addr points to the beginning of a block.
// Not meeting this contract will result in security holes and weird segfaults.
ProcId *k_findOwnerSlot(uint32_t addr) {
    uint32_t offset = addr - gMem.startMemoryAddress;
    uint32_t index = offset / gMem.blockSizeBytes;

    ProcId *header = (ProcId *)(offset - (offset % gMem.arenaSizeBytes));
    return &header[index];
}

// See note on k_findOwnerSlot
void k_setOwner(uint32_t addr, ProcId oid) {
    ProcId *ownerSlot = k_findOwnerSlot(addr);
    *ownerSlot = oid;
}

// See note on k_findOwnerSlot
ProcId k_getOwner(uint32_t addr) {
    return *k_findOwnerSlot(addr);
}

uint32_t k_getAlignedStartAddress(uint32_t start, uint32_t blockSizeBytes) {
    uint32_t offset = start % blockSizeBytes;
    if (offset == 0) {
        return start;
    }

    return start + (blockSizeBytes - offset);
}

// Only for use during initialization. Extracted for testing purposes.
void k_setGlobals(
    uint32_t startAddr,
    uint32_t endAddr,
    uint32_t blockSizeBytes
    ) {
    gMem.startMemoryAddress = k_getAlignedStartAddress(
        startAddr,
        blockSizeBytes
    );
    gMem.endMemoryAddress = endAddr;

    gMem.nextAvailableAddress = gMem.startMemoryAddress;
    gMem.blockSizeBytes = blockSizeBytes;
    gMem.arenaSizeBytes = blockSizeBytes * blockSizeBytes;
}

// Initialize global variables.
void k_memInit(void) {
    k_setGlobals(
        Image$$RW_IRAM1$$ZI$$Limit, // startAddr
        0x10008000,                 // endAddr
        1 << 7                      // blockSizeBytes = 128 bytes
    );
}

// Acquire a memory block. Will set the block's owner to the
// passed in owner id (oid).
void *k_acquireMemoryBlock(ProcId oid) {
    FreeBlock *curFirstFree = NULL;
    void *ret = NULL;
    ProcId *header = NULL;
    uint8_t didAllocateHeader = 0;
    uint32_t memOffset;

    // Try free list, first
    if (gMem.firstFree != NULL) {
        curFirstFree = gMem.firstFree;
        gMem.firstFree = curFirstFree->prev;
        ret = (void *)curFirstFree;
        k_setOwner((uint32_t)ret, oid);
        return ret;
    }

    // Leave room for owner list (header), if necessary
    memOffset = gMem.nextAvailableAddress - gMem.startMemoryAddress;
    if ((memOffset % gMem.arenaSizeBytes) == 0) {
        header = (ProcId *)gMem.nextAvailableAddress;
        didAllocateHeader = 1;
        gMem.nextAvailableAddress += gMem.blockSizeBytes;
    }

    // Check if we're out of memory
    // TODO: figure out what to return if OOM
    if (gMem.nextAvailableAddress >= gMem.endMemoryAddress) {
        return NULL;
    }

    // NOTE: this breaks if your memory starts at 0x0.
    if (didAllocateHeader) {
        *header = PROC_ID_KERNEL;
        ++header;
        while (header < (ProcId *)gMem.nextAvailableAddress) {
            *header = PROC_ID_NONE;
            ++header;
        }
    }

    ret = (void *)gMem.nextAvailableAddress;
    k_setOwner((uint32_t)ret, oid);
    gMem.nextAvailableAddress += gMem.blockSizeBytes;
    return ret;
}

int k_releaseMemoryBlock(void *mem, ProcId oid) {
    uint32_t addr;
    uint32_t addrOffset;
    uint32_t blockOffset;
    ProcId blockOwner;
    FreeBlock *fb;

    // Do a bunch of checks here
    addr = (uint32_t)mem;

    // First, check for obvious out of range errors
    if (addr < gMem.startMemoryAddress || addr >= gMem.endMemoryAddress) {
        return -1;
    }

    addrOffset = addr - gMem.startMemoryAddress;
    blockOffset = addrOffset % gMem.blockSizeBytes;

    // Disallow addresses in the middle of blocks
    if (blockOffset != 0) {
        return -1;
    }

    // Make sure this is allocated, and is owned by this process.
    blockOwner = k_getOwner(addr);
    if (blockOwner != oid) {
        return -1;
    }

    k_setOwner(addr, PROC_ID_NONE);

    // Add to free list
    fb = (FreeBlock *)mem;
    fb->prev = gMem.firstFree;
    gMem.firstFree = fb;
    return 0;
}
