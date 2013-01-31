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
uint32_t __SVC_0 _releaseProcessor(uint32_t k_func);

// setProcessPriority
extern uint32_t bridge_setProcessPriority(uint8_t pid, uint8_t priority);
#define set_process_priority(pid, priority) _setProcessPriority((uint32_t)bridge_setProcessPriority, pid, priority)
uint32_t __SVC_0 _setProcessPriority(uint32_t k_func, uint8_t pid, uint8_t priority);

// getProcessPriority
extern uint8_t bridge_getProcessPriority(uint8_t priority);
#define get_process_priority(pid) _getProcessPriority((uint32_t)bridge_getProcessPriority, pid)
uint8_t __SVC_0 _getProcessPriority(uint32_t k_func, uint8_t pid);


// acquireMemoryBlock
extern void *bridge_acquireMemoryBlock(void);
#define request_memory_block() _acquireMemoryBlock((uint32_t)bridge_acquireMemoryBlock)
uint32_t __SVC_0 _acquireMemoryBlock(uint32_t k_func);

// releaseMemoryBlock
extern uint32_t bridge_releaseMemoryBlock(void *blk);
#define release_memory_block(blk) _releaseMemoryBlock((uint32_t)bridge_releaseMemoryBlock, blk)
uint32_t __SVC_0 _releaseMemoryBlock(uint32_t k_func, void *blk);


#endif

