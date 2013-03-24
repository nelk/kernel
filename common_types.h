#ifndef COMMON_TYPES_H
#define COMMON_TYPES_H

#include <stddef.h>
#include <stdint.h>

// Error-codes

#define SUCCESS (0)
#define ENOMEM  (1)
#define EPERM   (2)
#define EINVAL  (3)

// Useful system pids
#define NULL_PID (0)
#define FIRST_USER_PID (1)
#define STRESS_A_PID (7)
#define STRESS_B_PID (8)
#define STRESS_C_PID (9)
#define CLOCK_PID (10)
#define CRT_PID (11)
#define KEYBOARD_PID (12)
#define SET_PRIORITY_PID (13)

// exposed so we can expose the size of messages in the envelopes
#define BLOCKSIZE_BYTES (1 << 7)
#define MESSAGEDATA_SIZE_BYTES (96)

#define MILLISECONDS_IN_SECOND  (1000)
#define SECONDS_IN_MINUTE       (60)
#define SECONDS_IN_HOUR         (3600)  // 60 * 60
#define SECONDS_IN_DAY          (86400) // 60 * 60 * 24

enum MessageType {
    MT_UNSET,
    MT_COUNT_REPORT,
    MT_SLEEP,
    MT_DEBUG,
    MT_KEYBOARD,
    MT_CRT_WAKEUP,
};

#define FOREGROUND_COLOR_BASE (128)
enum ForegroundColor {
    FC_BLACK = FOREGROUND_COLOR_BASE,
    FC_RED,
    FC_GREEN,
    FC_YELLOW,
    FC_BLUE,
    FC_MAGENTA,
    FC_CYAN,
    FC_WHITE,
};

#define BACKGROUND_COLOR_BASE (FC_WHITE+1)
enum BackgroundColor {
    BC_BLACK = BACKGROUND_COLOR_BASE,
    BC_RED,
    BC_GREEN,
    BC_YELLOW,
    BC_BLUE,
    BC_MAGENTA,
    BC_CYAN,
    BC_WHITE,
};


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
