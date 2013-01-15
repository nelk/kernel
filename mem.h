#ifndef MEM_H
#define MEM_H

#include <stdint.h>

extern uint32_t Image$$RW_IRAM1$$ZI$$Limit;

// TODO: move this to proc.h (which doesn't exist yet)
// ProcId is used to store pids and is typedef'd
// to distinguish it from regular integers.
typedef uint8_t ProcId;

#define PROC_ID_KERNEL (0x80)
#define PROC_ID_NONE   (0xff)

typedef struct FreeBlockStruct FreeBlock;

struct FreeBlockStruct {
    FreeBlock *prev;
};

typedef struct {
    uint32_t startMemoryAddress;
    uint32_t endMemoryAddress;

    uint32_t nextAvailableAddress;

    uint32_t blockSizeBytes;
    uint32_t arenaSizeBytes;

    FreeBlock *firstFree;
} MemInfo;

extern MemInfo gMem;

void k_memInit(void);
void *k_acquireMemoryBlock(ProcId oid);
int k_releaseMemoryBlock(void *mem, ProcId oid);


#ifdef TESTING

void k_setOwner(uint32_t addr, ProcId oid);
ProcId k_getOwner(uint32_t addr);
uint32_t k_getAlignedStartAddress(uint32_t startAddr, uint32_t blockSizeBytes);
void k_setGlobals(
    uint32_t startAddr,
    uint32_t endAddr,
    uint32_t blockSizeBytes
);

#endif

#endif // MEM_H
