#include "type.h"
#include "mem.h"

extern uint32_t Image$$RW_IRAM1$$ZI$$Limit;

typedef struct FreeBlockStruct FreeBlock;

struct FreeBlockStruct {
	FreeBlock *prev;
};

typedef struct {
	uint32_t blockSizeBytes;

	uint32_t startMemoryAddress;
	uint32_t endMemoryAddress;

	uint32_t nextAvailableAddress;

	FreeBlock *firstFree;
} MemInfo;

MemInfo gMem;

uint32_t k_getAlignedStartAddress(uint32_t start, uint32_t blockSizeBytes) {
	uint32_t offset = start % blockSizeBytes;
	return start + (blockSizeBytes - offset);
}

void k_memInit(void) {
	gMem.blockSizeBytes = 1 << 7; // 128 bytes

	gMem.startMemoryAddress = k_getAlignedStartAddress(
		Image$$RW_IRAM1$$ZI$$Limit,
		gMem.blockSizeBytes
	);
	gMem.endMemoryAddress = 0x10008000;

	gMem.nextAvailableAddress = gMem.startMemoryAddress;
}

void *k_acquireMemoryBlock(void) {
	FreeBlock *curFirstFree;
	void *ret;

	// Try free list, first
	if (gMem.firstFree != NULL) {
		curFirstFree = gMem.firstFree;
		gMem.firstFree = curFirstFree->prev;
		ret = (void *)curFirstFree;
		return ret;
	}

	// Out of memory
	// TODO: figure out what to return if OOM
	if (gMem.nextAvailableAddress == gMem.endMemoryAddress) {
		return NULL;
	}

	ret = (void *)gMem.nextAvailableAddress;
	gMem.nextAvailableAddress += gMem.blockSizeBytes;
	return ret;
}

int k_releaseMemoryBlock(void *mem) {
	// TODO: check if calling process owns this block
	// BUG: this allows double-freeing

	uint32_t addr;
	uint32_t addrOffset;
	uint32_t blockOffset;
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

	// Add to free list
	fb = (FreeBlock *)mem;
	fb->prev = gMem.firstFree;
	gMem.firstFree = fb;
	return 0;
}
