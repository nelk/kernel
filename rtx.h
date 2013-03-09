#ifndef RTX_H
#define RTX_H

#include <stdint.h>

#include "common_types.h"

/**
 * User Land API
 *
 * uint32_t release_processor(void)
 * uint32_t set_process_priority(uint8_t pid, uint8_t priority)
 * uint8_t get_process_priority(uint8_t pid)
 *
 * void *request_memory_block(void)
 * uint32_t release_memory_block(void *blk)
 * uint8_t is_out_of_memory(void)
 *
 * int8_t send_message(uint8_t pid, Envelope *envelope)
 * Envelope *received_message(uint8_t *senderPid) // output parameter
 * int8_t delayed_send(uint8_t pid, Envelope *envelope, uint32_t delay)
 *
 * uint32_t getTime(void)
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
extern int16_t bridge_getProcessPriority(uint8_t priority);
#define get_process_priority(pid) _getProcessPriority((uint32_t)bridge_getProcessPriority, pid)
int16_t __SVC_0 _getProcessPriority(uint32_t k_func, uint8_t pid);


// tryAcquireMemoryBlock
extern void *bridge_tryAcquireMemoryBlock(void);
#define try_request_memory_block() ((void*)(_tryAcquireMemoryBlock((uint32_t)bridge_tryAcquireMemoryBlock)))
uint32_t __SVC_0 _tryAcquireMemoryBlock(uint32_t k_func);

// acquireMemoryBlock
extern void *bridge_acquireMemoryBlock(void);
#define request_memory_block() ((void*)(_acquireMemoryBlock((uint32_t)bridge_acquireMemoryBlock)))
__SVC_0 _acquireMemoryBlock(uint32_t k_func);

// releaseMemoryBlock
extern int8_t bridge_releaseMemoryBlock(void *blk);
#define release_memory_block(blk) _releaseMemoryBlock((uint32_t)bridge_releaseMemoryBlock, blk)
int8_t __SVC_0 _releaseMemoryBlock(uint32_t k_func, void *blk);


// sendMessage
extern int8_t bridge_sendMessage(uint8_t pid, Envelope *envelope);
#define send_message(pid, envelope) _sendMessage((uint32_t)bridge_sendMessage, pid, envelope)
int8_t __SVC_0 _sendMessage(uint32_t k_func, uint8_t pid, Envelope *envelope);

// receiveMessage
extern Envelope *bridge_receiveMessage(uint8_t *senderPid);
#define receive_message(senderPid) ((Envelope*)(_receiveMessage((uint32_t)bridge_sendMessage, senderPid)))
__SVC_0 _receiveMessage(uint32_t k_func, uint8_t *senderPid);

// delayedSend
extern int8_t bridge_sendDelayedMessage(uint8_t pid, Envelope *envelope, uint32_t delay);
#define delayed_send(pid, envelope, delay) _delayedSend((uint32_t)bridge_sendDelayedMessage, pid, envelope, delay)
int8_t __SVC_0 _delayedSend(uint32_t k_func, uint8_t pid, Envelope *envelope, uint32_t delay);


// get_time()
extern uint32_t bridge_getTime(void);
#define get_time() _getTime((uint32_t)bridge_getTime)
uint32_t __SVC_0 _getTime(uint32_t k_func);

// pid()
extern ProcId bridge_getPid(void);
#define pid() _getPid((uint32_t)bridge_getPid)
ProcId __SVC_0 _getPid(uint32_t k_func);

#endif
