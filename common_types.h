#ifndef COMMON_TYPES
#define COMMON_TYPES

// Useful system pids
#define CLOCK_PID (1)
#define CRT_PID (2)
#define KEYBOARD_PID (3)

// exposed so we can expose the size of messages in the envelopes
#define BLOCKSIZE_BYTES (1 << 7)
#define MESSAGEDATA_SIZE_BYTES (96)

// ProcId is used to store pids and is typedef'd
// to distinguish it from regular integers.
typedef uint8_t ProcId;

// TODO(alex) - move Envelope to user-facing header (user_message.h) or aggregate user-facing types header.
typedef struct Envelope Envelope;
struct Envelope {
    Envelope *next;

    uint32_t sendTime;

    ProcId srcPid;
    ProcId dstPid;

    uint32_t messageType;
    char messageData[MESSAGEDATA_SIZE_BYTES];
};

#endif
