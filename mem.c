#include <stddef.h>

#include "mem.h"

// This function assumes addr points to the beginning of a block.
// Not meeting this contract will result in security holes and weird segfaults.
ProcId *k_findOwnerSlot(MemInfo *memInfo, uint32_t addr) {
    uint32_t offset = addr - memInfo->startMemoryAddress;
    uint32_t index = (offset / memInfo->blockSizeBytes) % memInfo->blockSizeBytes;

    ProcId *header = (ProcId *)(offset - (offset % memInfo->arenaSizeBytes));
    return header + index + memInfo->startMemoryAddress;
}

// See note on k_findOwnerSlot
void k_setOwner(MemInfo *memInfo, uint32_t addr, ProcId oid) {
    ProcId *ownerSlot = k_findOwnerSlot(memInfo, addr);
    *ownerSlot = oid;
}

// Checks if addr is owned by oid, see note on k_findOwnerSlot
uint8_t k_isOwner(MemInfo *memInfo, uint32_t addr, ProcId oid) {
    return (*k_findOwnerSlot(memInfo, addr) == oid);
}

uint32_t k_getAlignedStartAddress(uint32_t start, uint32_t blockSizeBytes) {
    uint32_t offset = start % blockSizeBytes;
    if (offset == 0) {
        return start;
    }

    return start + (blockSizeBytes - offset);
}

// Only for use during initialization. Extracted for testing purposes.
// Note that blockSizeBytes must be greater than or equal to
// sizeof(FreeBlock), or else madness will ensue.
void k_memInfoInit(
    MemInfo *memInfo,
    uint32_t startAddr,
    uint32_t endAddr,
    uint32_t blockSizeBytes
    ) {
    memInfo->startMemoryAddress = k_getAlignedStartAddress(
        startAddr,
        blockSizeBytes
    );
    memInfo->endMemoryAddress = endAddr;

    memInfo->nextAvailableAddress = memInfo->startMemoryAddress;
    memInfo->blockSizeBytes = blockSizeBytes;
    memInfo->arenaSizeBytes = blockSizeBytes * blockSizeBytes;
    memInfo->firstFree = NULL;
}

// Acquire a memory block. Will set the block's owner to the
// passed in owner id (oid).
void *k_acquireMemoryBlock(MemInfo *memInfo, ProcId oid) {
    FreeBlock *curFirstFree;
    void *ret;
    ProcId *header;
    uint8_t didAllocateHeader;
    uint32_t memOffset;

    curFirstFree = NULL;
    ret = NULL;
    header = NULL;
    didAllocateHeader = 0;
    memOffset = 0;

    // Try free list, first
    if (memInfo->firstFree != NULL) {
        curFirstFree = memInfo->firstFree;
        memInfo->firstFree = curFirstFree->prev;
        ret = (void *)curFirstFree;
        k_setOwner(memInfo, (uint32_t)ret, oid);
        return ret;
    }

    // Leave room for owner list (header), if necessary
    memOffset = memInfo->nextAvailableAddress - memInfo->startMemoryAddress;
    if ((memOffset % memInfo->arenaSizeBytes) == 0) {
        header = (ProcId *)memInfo->nextAvailableAddress;
        didAllocateHeader = 1;
        memInfo->nextAvailableAddress += memInfo->blockSizeBytes;
    }

    // Check if we're out of memory
    if (memInfo->nextAvailableAddress >= memInfo->endMemoryAddress) {
        return NULL;
    }

    // NOTE: this breaks if your memory starts at 0x0.
    if (didAllocateHeader) {
        *header = PROC_ID_ALLOCATOR;
        ++header;
        while (header < (ProcId *)memInfo->nextAvailableAddress) {
            *header = PROC_ID_NONE;
            ++header;
        }
    }

    ret = (void *)memInfo->nextAvailableAddress;
    k_setOwner(memInfo, (uint32_t)ret, oid);
    memInfo->nextAvailableAddress += memInfo->blockSizeBytes;

    return ret;
}

int8_t k_releaseMemoryBlock(
    MemInfo *memInfo,
    void *mem,
    ProcId oid) {

    uint32_t addr;
    uint32_t addrOffset;
    uint32_t blockOffset;
    FreeBlock *fb;

    addr = (uint32_t)mem;

    // First, check for obvious out of range errors.
    // NOTE: nextAvailableAddress is always greater than or
    //       equal to endMemoryAddress, but we check both
    //       anyways.
    if (
        addr < memInfo->startMemoryAddress ||
        addr >= memInfo->nextAvailableAddress ||
        addr >= memInfo->endMemoryAddress
    ) {
        return ERR_OUTOFRANGE;
    }

    addrOffset = addr - memInfo->startMemoryAddress;
    blockOffset = addrOffset % memInfo->blockSizeBytes;

    // Disallow addresses in the middle of blocks
    if (blockOffset != 0) {
        return ERR_UNALIGNED;
    }

    // Make sure this is allocated, and is owned by this process.
    if (!k_isOwner(memInfo, addr, oid)) {
        return ERR_PERM;
    }

    k_setOwner(memInfo, addr, PROC_ID_NONE);

    // Add to free list
    fb = (FreeBlock *)mem;
    fb->prev = memInfo->firstFree;
    memInfo->firstFree = fb;

    return SUCCESS;
}
