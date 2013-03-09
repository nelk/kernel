#ifndef COMMON_TYPES_H
#define COMMON_TYPES_H

// Error-codes

#define SUCCESS (0)
#define ENOMEM  (1)
#define EPERM   (2)
#define EINVAL  (3)

// Useful system pids
#define CLOCK_PID (1)
#define CRT_PID (2)
#define KEYBOARD_PID (3)

// exposed so we can expose the size of messages in the envelopes
#define BLOCKSIZE_BYTES (1 << 7)
#define MESSAGEDATA_SIZE_BYTES (96)

#define MILLISECONDS_IN_SECOND  (1000)
#define SECONDS_IN_MINUTE       (60)
#define SECONDS_IN_HOUR         (3600)  // 60 * 60
#define SECONDS_IN_DAY          (86400) // 60 * 60 * 24

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
