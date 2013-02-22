#ifndef MEM_H
#define MEM_H

#include <stdint.h>

#include "proc.h"


typedef struct FreeBlock FreeBlock;
struct FreeBlock {
    FreeBlock *prev;
};

typedef struct MemInfo MemInfo;
struct MemInfo {
    uint32_t startMemoryAddress;
    uint32_t endMemoryAddress;

    uint32_t nextAvailableAddress;

    uint32_t blockSizeBytes;
    uint32_t arenaSizeBytes;

    FreeBlock *firstFree;
};

#define SUCCESS (0)
#define ERR_OUTOFRANGE (-1)
#define ERR_UNALIGNED (-2)
#define ERR_PERM (-3)

void *k_acquireMemoryBlock(MemInfo *memInfo, ProcId oid);
int8_t k_releaseMemoryBlock(
    MemInfo *memInfo,
    void *mem,
    ProcId oid
);
void k_memInfoInit(
    MemInfo *memInfo,
    uint32_t startAddr,
    uint32_t endAddr,
    uint32_t blockSizeBytes
);


#ifdef TESTING
ProcId *k_findOwnerSlot(MemInfo *memInfo, uint32_t addr);
void k_setOwner(MemInfo *memInfo, uint32_t addr, ProcId oid);
uint8_t k_isOwner(MemInfo *memInfo, uint32_t addr, ProcId oid);
uint32_t k_getAlignedStartAddress(uint32_t startAddr, uint32_t blockSizeBytes);
#endif

#endif // MEM_H
