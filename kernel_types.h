#ifndef KERNEL_TYPES_H
#define KERNEL_TYPES_H

#include <stdint.h>

#include "common_types.h"
#include "heap.h"

// Keil Related Types
typedef int32_t ssize_t;

// Keyboard Hotkey related consts. This will break everything if it is '%'.
#define SHOW_DEBUG_PROCESSES ('!')

#define UART_OUTPUT_BUFSIZE (16)

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

    // accounting info
    uint32_t numSuccessfulAllocs;
    uint32_t numFailedAllocs;
    uint32_t numFreeCalls;
};


// Process-control related types

#define NUM_PROCS (14)

#define PROC_ID_KERNEL    (0x80)
#define PROC_ID_ALLOCATOR (0x81)
#define PROC_ID_NONE      (0xff)

#define UART_IN_BUF_SIZE (64)

enum ProcState {
		// TODO(nelk): prepend PS_ to these so we don't have a global 'NEW' defined here
    BLOCKED_MEMORY,
    BLOCKED_MESSAGE,
    NEW,
    READY,
    RUNNING,
		UNUSED,
};
typedef enum ProcState ProcState;

#define USER_PRIORITY_MASK ((uint32_t)0xFF)
#define KERN_PRIORITY_MASK (~(USER_PRIORITY_MASK))
#define KERN_PRIORITY_SHIFT (8)
#define MAX_PRIORITY (USER_PRIORITY_MASK)

typedef struct PCB PCB;
struct PCB {
    uint32_t *stack;
    ProcId pid;
    ProcState state;
    uint32_t *startLoc;

    uint32_t priority;

    ssize_t rqIndex;
    ssize_t memqIndex;

    struct Envelope *mqHead;
    struct Envelope *mqTail;

    struct Envelope *debugEnv;
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

#define CRT_OUTQ_LEN (80)
#define CRT_LINE_LIMIT (80)

typedef struct CRTData CRTData;
struct CRTData {
    uint8_t outqBuf[CRT_OUTQ_LEN];
    uint8_t outqWriter;
    uint8_t outqReader;

    uint8_t readIndex;
    Envelope *envqHead;
    Envelope *envqTail;

    Envelope *freeList;

    // The position of the cursor on the screen
    // Format of screenCursorPos is as follows:
    // Highest order bit (i.e. (screenCursorPos >> 7)) stores whether its
    // on the user line (0) or on the process line (1).
    // The remaining 7 bits (i.e. screenCursorPos & 0x7f) stores where on that
    // line the cursor is.
    uint8_t screenCursorPos;

    // The horizontal position of the cursor in the process line
    uint8_t procCursorPos;

    // The last process to print something
    ProcId cursorOwner;

    // The line buffer is our internal model of what the user has typed
    uint8_t lineBuf[CRT_LINE_LIMIT];
    uint8_t lineBufLen;

    // The screen buffer is what is currently on the screen on the user line
    uint8_t screenBuf[CRT_LINE_LIMIT]
    uint8_t screenBufLen;

    // This index answers the question "if the user typed a character, where in
    // lineBuf would that character be inserted?"
    uint8_t userCursorPos;
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

    CRTData crtData;
    Envelope *uartOutputEnv;

    // UART keyboard input data
    char inputBuf[UART_IN_BUF_SIZE];
    uint8_t debugSem;
    volatile uint32_t readIndex; // Next read index
    volatile uint32_t writeIndex; // Next write index
    volatile uint32_t inputBufOverflow;
    Envelope *currentEnv; // This is initialized to new block
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

// Interrupt process-related types

typedef struct ClockInfo ClockInfo;
struct ClockInfo {
    uint32_t totalTime;
};

#endif
