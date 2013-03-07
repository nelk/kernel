#include <stddef.h>

#include "mem.h"


int8_t k_validMemoryBlock(MemInfo *memInfo, uint32_t addr, ProcId oid) {
    uint32_t addrOffset;
    uint32_t blockOffset;

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

    return SUCCESS;
}

// This function assumes addr points to the beginning of a block.
// Not meeting this contract will result in security holes and weird segfaults.
ProcId *k_findOwnerSlot(MemInfo *memInfo, uint32_t addr) {
    uint32_t offset = addr - memInfo->startMemoryAddress;
    uint32_t index = (offset / memInfo->blockSizeBytes) % memInfo->blockSizeBytes;

    ProcId *header = (ProcId *)(offset - (offset % memInfo->arenaSizeBytes));
    return header + index + memInfo->startMemoryAddress;
}

// See note on k_findOwnerSlot
int8_t k_changeOwner(MemInfo *memInfo, uint32_t addr, ProcId oid) {
    int8_t isValid = k_validMemoryBlock(memInfo, addr, oid);
    if (isValid != SUCCESS) {
        return isValid;
    }

    ProcId *ownerSlot = NULL;
    if (!(memInfo->trackOwners)) {
        return 1;
    }
    ownerSlot = k_findOwnerSlot(memInfo, addr);
    *ownerSlot = oid;
    return SUCCESS;
}

// Checks if addr is owned by oid, see note on k_findOwnerSlot
uint8_t k_isOwner(MemInfo *memInfo, uint32_t addr, ProcId oid) {
    if (!(memInfo->trackOwners)) {
        return 1;
    }
    return (*k_findOwnerSlot(memInfo, addr) == oid);
}

uint32_t k_getAlignedStartAddress(uint32_t start, uint32_t blockSizeBytes) {
    uint32_t offset = start % blockSizeBytes;
    if (offset == 0) {
        return start;
    }

    return start + (blockSizeBytes - offset);
}

uint32_t k_getAlignedEndAddress(uint32_t end, uint32_t blockSizeBytes) {
    return end - (end % blockSizeBytes);
}

// Only for use during initialization. Extracted for testing purposes.
// Note that blockSizeBytes must be greater than or equal to
// sizeof(FreeBlock), or else madness will ensue.
void k_memInfoInit(
    MemInfo *memInfo,
    uint32_t startAddr,
    uint32_t endAddr,
    uint32_t blockSizeBytes,
    uint8_t trackOwners
    ) {
    memInfo->startMemoryAddress = k_getAlignedStartAddress(
        startAddr,
        blockSizeBytes
    );
    memInfo->endMemoryAddress = k_getAlignedEndAddress(
        endAddr,
        blockSizeBytes
    );

    memInfo->nextAvailableAddress = memInfo->startMemoryAddress;
    memInfo->blockSizeBytes = blockSizeBytes;
    memInfo->arenaSizeBytes = blockSizeBytes * blockSizeBytes;
    memInfo->firstFree = NULL;
    memInfo->trackOwners = trackOwners;
}

// Acquire a memory block. Will set the block's owner to the
// passed in owner id (oid).
void *k_acquireMemoryBlock(MemInfo *memInfo, ProcId oid) {
    FreeBlock *curFirstFree = NULL;
    void *ret = NULL;
    ProcId *header = NULL;
    uint8_t didAllocateHeader = 0;
    uint32_t memOffset = 0;

    // Try free list, first
    if (memInfo->firstFree != NULL) {
        curFirstFree = memInfo->firstFree;
        memInfo->firstFree = curFirstFree->prev;
        ret = (void *)curFirstFree;
        k_changeOwner(memInfo, (uint32_t)ret, oid);
        return ret;
    }

    // Leave room for owner list (header), if necessary
    memOffset = memInfo->nextAvailableAddress - memInfo->startMemoryAddress;
    if (memInfo->trackOwners && (memOffset % memInfo->arenaSizeBytes) == 0) {
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
    k_changeOwner(memInfo, (uint32_t)ret, oid);
    memInfo->nextAvailableAddress += memInfo->blockSizeBytes;

    return ret;
}

int8_t k_releaseMemoryBlock(MemInfo *memInfo, uint32_t addr, ProcId oid) {
    FreeBlock *fb = NULL;

    // TODO(sanjay): not validating that addr is owned by oid

    int8_t isValid = k_changeOwner(memInfo, addr, PROC_ID_NONE);
    if (isValid != SUCCESS) {
        return isValid;
    }

    // Add to free list
    fb = (FreeBlock *)addr;
    fb->prev = memInfo->firstFree;
    memInfo->firstFree = fb;

    return SUCCESS;
}
