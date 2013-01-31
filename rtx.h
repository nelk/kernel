#ifndef RTX_H
#define RTX_H

#include <stdint.h>

/**
 * User Land API
 *
 * uint32_t release_processor(void)
 * uint32_t set_process_priority(int8_t pid, int8_t priority)
 * uint8_t get_process_priority(int8_t pid)
 *
 * void *request_memory_block(void)
 * uint32_t release_memory_block(void *blk)
 */

#define __SVC_0  __svc_indirect(0)

// releaseProcessor
extern uint32_t bridge_releaseProcessor(void);
#define release_processor() _releaseProcessor((uint32_t)bridge_releaseProcessor)
extern uint32_t _releaseProcessor(uint32_t k_func) __SVC_0

// setProcessPriority
extern uint32_t bridge_setProcessPriority(uint8_t pid, uint8_t priority);
#define set_process_priority(pid, priority) _setProcessPriority((uint32_t)bridge_setProcessPriority, pid, priority)
extern uint32_t _setProcessPriority(uint32_t k_func, uint8_t pid, uint8_t priority) __SVC_0

// getProcessPriority
extern uint8_t bridge_getProcessPriority(uint8_t priority);
#define get_process_priority(pid) _getProcessPriority((uint32_t)bridge_getProcessPriority, pid)
extern uint8_t _getProcessPriority(uint32_t k_func, uint8_t pid) __SVC_0


// acquireMemoryBlock
extern void *bridge_acquireMemoryBlock(void);
#define request_memory_block() _acquireMemoryBlock((uint32_t)bridge_acquireMemoryBlock)
extern uint32_t _acquireMemoryBlock(uint32_t k_func) __SVC_0

// releaseMemoryBlock
extern uint32_t bridge_releaseMemoryBlock(void *blk);
#define release_memory_block(blk) _releaseMemoryBlock((uint32_t)bridge_releaseMemoryBlock, blk)
extern uint32_t _releaseMemoryBlock(uint32_t k_func, void *blk) __SVC_0


#endif

