#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>

#include "common.h"
#include "heap.h"

// Keil Related Types
typedef uint32_t ssize_t;

// Memory-related types

struct FreeBlock;

typedef struct MemInfo MemInfo;
struct MemInfo {
    uint32_t startMemoryAddress;
    uint32_t endMemoryAddress;

    uint32_t nextAvailableAddress;

    uint32_t blockSizeBytes;
    uint32_t arenaSizeBytes;

    uint8_t trackOwners;

    struct FreeBlock *firstFree;
};


// Process-control related types

#define NUM_PROCS (6)
#define MAX_PRIORITY (4)

// ProcId is used to store pids and is typedef'd
// to distinguish it from regular integers.
typedef uint8_t ProcId;

#define PROC_ID_KERNEL    (0x80)
#define PROC_ID_ALLOCATOR (0x81)
#define PROC_ID_NONE      (0xff)

enum ProcState {
    BLOCKED,
    BLOCKED_MESSAGE,
    NEW,
    READY,
    RUNNING,
};
typedef enum ProcState ProcState;

struct Envelope;

typedef struct PCB PCB;
struct PCB {
    uint32_t *stack;
    ProcId pid;
    ProcState state;
    uint32_t *startLoc;

    uint32_t priority;

    ssize_t rqIndex;
    ssize_t memqIndex;

    struct Envelope *messageQueue;
    struct Envelope *endOfMessageQueue;
};

typedef struct PQEntry PQEntry;
struct PQEntry {
    PCB *pcb;
    uint32_t seqNumber;
};

typedef ssize_t *(*storeIndexFunc)(PCB *);

typedef struct PQ PQ;
struct PQ {
    heap storeMgr;
    PQEntry *store;
    size_t size;
    size_t cap;

    uint32_t seq;

    storeIndexFunc getIndexInStore; // takes a PCB* and returns a ssize_t*
};

typedef struct ProcInfo ProcInfo;
struct ProcInfo {
    PCB processes[NUM_PROCS]; // Actual process blocks
    PCB *currentProcess;
    PCB *nullProcess;

    PQ prq; // Process ready queue
    PQEntry procQueue[NUM_PROCS];

    PQ memq; // Memory blocked queue
    PQEntry memQueue[NUM_PROCS];
};

enum ReleaseReason {
    CHANGED_PRIORITY,
    MEMORY_FREED,
    MESSAGE_RECEIVE,
    MESSAGE_SENT,
    OOM,
    YIELD,
};
typedef enum ReleaseReason ReleaseReason;

// Message - based:

// TODO(alex) - move Envelope to user-facing header (user_message.h) or aggregate user-facing types header.
typedef struct Envelope Envelope;
struct Envelope {
    uint32_t header[3];

    // User Data
    uint32_t senderPid;
    uint32_t destPid;
    uint32_t messageType;
    char messageData[BLOCKSIZE_BYTES - 6*sizeof(uint32_t)];
};

typedef struct MessagePQ MessagePQ;
struct MessagePQ {
    heap storeMgr;
    Envelope **store;
    size_t size;
    size_t cap;

    uint32_t seq;
};

typedef struct MessageInfo MessageInfo;
struct MessageInfo {
    MessagePQ mpq; // Delayed Message PQ
    Envelope *messageStore[500];
};
#endif
