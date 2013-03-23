#include <stddef.h>

#include "helpers.h"
#include "mem.h"


int8_t k_validMemoryBlock(MemInfo *memInfo, uint32_t addr) {
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
        return EINVAL;
    }

    addrOffset = addr - memInfo->startMemoryAddress;
    blockOffset = addrOffset % memInfo->blockSizeBytes;

    // Disallow addresses in the middle of blocks
    if (blockOffset != 0) {
        return EINVAL;
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

// Checks if addr is owned by oid, see note on k_findOwnerSlot
// Assumes addr is owned by this memInfo, hence unsafe.
uint8_t k_isOwnerUnsafe(MemInfo *memInfo, uint32_t addr, ProcId oid) {
    if (!(memInfo->trackOwners)) {
        // TODO: constantify
        return 1;
    }
    return (*k_findOwnerSlot(memInfo, addr) == oid);
}

void k_setOwnerUnsafe(MemInfo *memInfo, uint32_t addr, ProcId newOid) {
    ProcId *ownerSlot = NULL;
    if (!memInfo->trackOwners) {
        return;
    }
    ownerSlot = k_findOwnerSlot(memInfo, addr);
    *ownerSlot = newOid;
}

int8_t k_changeOwner(
        MemInfo *memInfo,
        uint32_t addr,
        ProcId fromOid,
        ProcId toOid
        ) {
    int8_t isValid = k_validMemoryBlock(memInfo, addr);
    if (isValid != SUCCESS) {
        return isValid;
    }

    if (!k_isOwnerUnsafe(memInfo, addr, fromOid)) {
        return EPERM;
    }

    k_setOwnerUnsafe(memInfo, addr, toOid);
    return SUCCESS;
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
// sizeof(FreeBlock), or madness will ensue.
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
    memInfo->numSuccessfulAllocs = 0;
    memInfo->numFailedAllocs = 0;
    memInfo->numFreeCalls = 0;
}

// Acquire a memory block. Will set the block's owner to the
// passed in owner id (oid).
uint32_t k_acquireMemoryBlock(MemInfo *memInfo, ProcId oid) {
    FreeBlock *curFirstFree = NULL;
    uint8_t didAllocateHeader = 0;
    ProcId *header = NULL;
    uint32_t memOffset = 0;
    uint32_t ret = 0;

    // Try free list, first
    if (memInfo->firstFree != NULL) {
        curFirstFree = memInfo->firstFree;
        memInfo->firstFree = curFirstFree->prev;
        ret = (uint32_t)curFirstFree;
        // It's on free list, we can assume it's a legitimate address
        k_setOwnerUnsafe(memInfo, ret, oid);
        ++(memInfo->numSuccessfulAllocs);
        memset((uint8_t *)ret, memInfo->blockSizeBytes, 0);
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
        // TODO(sanjay): this is incredibly unsemantic,
        // use a bool "out_of_memory" out param instead
        ++(memInfo->numFailedAllocs);
        return 0;
    }

    if (didAllocateHeader) {
        memset(header, memInfo->blockSizeBytes, PROC_ID_NONE);
        *header = PROC_ID_ALLOCATOR;
    }

    ++(memInfo->numSuccessfulAllocs);

    ret = memInfo->nextAvailableAddress;
    k_setOwnerUnsafe(memInfo, ret, oid);
    memInfo->nextAvailableAddress += memInfo->blockSizeBytes;
    memset((uint8_t *)ret, memInfo->blockSizeBytes, 0);

    return ret;
}

int8_t k_releaseMemoryBlock(MemInfo *memInfo, uint32_t addr, ProcId oid) {
    FreeBlock *fb = NULL;

    // Change owner from oid -> PROC_ID_NONE, this will validate
    // that oid currently owns addr
    int8_t isValid = k_changeOwner(memInfo, addr, oid, PROC_ID_NONE);
    if (isValid != SUCCESS) {
        return isValid;
    }

    ++(memInfo->numFreeCalls);

    // Add to free list
    fb = (FreeBlock *)addr;
    fb->prev = memInfo->firstFree;
    memInfo->firstFree = fb;

    return SUCCESS;
}
