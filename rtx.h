#ifndef RTX_H
#define RTX_H

#include <stdint.h>

#define __SVC_0  __svc_indirect(0)

// releaseProcessor
extern uint32_t bridge_releaseProcessor(void);
#define release_processor() _releaseProcessor((uint32_t)bridge_releaseProcessor)
extern uint32_t _releaseProcessor(uint32_t k_func) __SVC_0


// acquireMemoryBlock
extern void *bridge_acquireMemoryBlock(void);
#define request_memory_block() _acquireMemoryBlock((uint32_t)bridge_acquireMemoryBlock)
extern uint32_t _acquireMemoryBlock(uint32_t k_func) __SVC_0

// releaseMemoryBlock
extern uint32_t bridge_releaseMemoryBlock(void *blk);
#define release_memory_block(blk) _releaseMemoryBlock((uint32_t)bridge_releaseMemoryBlock, blk)
extern uint32_t _releaseMemoryBlock(uint32_t k_func, void *blk) __SVC_0


#endif

