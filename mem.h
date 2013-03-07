#ifndef MEM_H
#define MEM_H

#include <stdint.h>

#include "types.h"
#include "proc.h"

typedef struct FreeBlock FreeBlock;
struct FreeBlock {
    FreeBlock *prev;
};

#define SUCCESS (0)
#define ERR_OUTOFRANGE (-1)
#define ERR_UNALIGNED (-2)
#define ERR_PERM (-3)

void *k_acquireMemoryBlock(MemInfo *memInfo, ProcId oid);
int8_t k_releaseMemoryBlock(
    MemInfo *memInfo,
    uint32_t addr,
    ProcId oid
);
void k_memInfoInit(
    MemInfo *memInfo,
    uint32_t startAddr,
    uint32_t endAddr,
    uint32_t blockSizeBytes,
    uint8_t trackOwners
);

int8_t k_changeOwner(MemInfo *memInfo, uint32_t addr, ProcId oid);
uint8_t k_isOwner(MemInfo *memInfo, uint32_t addr, ProcId oid);

#ifdef TESTING
int8_t k_validMemoryBlock(MemInfo *memInfo, void *mem, ProcId oid);
ProcId *k_findOwnerSlot(MemInfo *memInfo, uint32_t addr);
uint32_t k_getAlignedStartAddress(uint32_t startAddr, uint32_t blockSizeBytes);
#endif

#endif // MEM_H
