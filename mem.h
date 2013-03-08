#ifndef MEM_H
#define MEM_H

#include <stdint.h>

#include "types.h"
#include "proc.h"

typedef struct FreeBlock FreeBlock;
struct FreeBlock {
    FreeBlock *prev;
};

uint32_t k_acquireMemoryBlock(MemInfo *memInfo, ProcId oid);
int8_t k_releaseMemoryBlock(
    MemInfo *memInfo,
    uint32_t addr,
    ProcId oid
);

// Changes owner of addr from fromOid to toOid. Validates addr.
int8_t k_changeOwner(
    MemInfo *memInfo,
    uint32_t addr,
    ProcId fromOid,
    ProcId toOid
);

void k_memInfoInit(
    MemInfo *memInfo,
    uint32_t startAddr,
    uint32_t endAddr,
    uint32_t blockSizeBytes,
    uint8_t trackOwners
);

#ifdef TESTING
uint8_t k_isOwnerUnsafe(MemInfo *memInfo, uint32_t addr, ProcId oid);
void k_setOwnerUnsafe(MemInfo *memInfo, uint32_t addr, ProcId newOid);

ProcId *k_findOwnerSlot(MemInfo *memInfo, uint32_t addr);

uint32_t k_getAlignedStartAddress(uint32_t startAddr, uint32_t blockSizeBytes);
uint32_t k_getAlignedEndAddress(uint32_t end, uint32_t blockSizeBytes);

int8_t k_validMemoryBlock(MemInfo *memInfo, void *mem, ProcId oid);
#endif

#endif // MEM_H
