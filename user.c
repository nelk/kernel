#include <stddef.h>

#include "rtx.h"
#include "uart_polling.h"
#include "user.h"
#include "helpers.h"

// User Land

void sleep(uint32_t ms, Envelope **listEnv, Envelope *sleepEnv) {
#if 0
    uint32_t currentTime = get_time();
    uint32_t targetTime = currentTime + ms;

    while (sleepEnv == NULL && currentTime < targetTime) {
        release_processor();
        sleepEnv = (Envelope *)try_request_memory_block();
        currentTime = get_time();
    }

    if (currentTime >= targetTime) {
        release_memory_block((void *)sleepEnv); // Won't release if null.
        return;
    }
#else
    uint32_t currentTime = get_time();
    uint32_t targetTime = currentTime + ms;
#endif
    sleepEnv->messageType = MT_SLEEP;
    delayed_send(pid(), sleepEnv, targetTime - currentTime);
    sleepEnv = NULL;

    // If the user passed us a message queue, advance it as far as possible
    // so we're writing to the back of that queue.
    if (listEnv != NULL) {
        while (*listEnv != NULL) {
            listEnv = &((*listEnv)->next);
        }
    }

    while (1) {
        Envelope *env = receive_message(NULL);

        // If this is a wake-up message, then just release it and break.
        // Otherwise we need to save this message. If the user passed us a
        // queue use that, otherwise just send it to ourself on a delay.
        // If we send it to ourself, we lose srcPid.
        if (env->messageType == MT_SLEEP) {
            break;
        } else if (listEnv != NULL) {
            env->next = NULL;
            *listEnv = env;
            listEnv = &(env->next);
        } else {
            // TODO(sanjay): we don't need to delay it this much, we could delay
            // it less.
            delayed_send(pid(), env, ms);
        }
    }
}


void nullProcess(void) {
    while (1) {
        release_processor();
    }
}

void printProcess(char *c) {
    while (1) {
        Envelope *envelope = (Envelope *)request_memory_block();
        uint8_t i = 0;
        uint8_t bufLen = MESSAGEDATA_SIZE_BYTES - 1; // -1 for null byte

        i += write_string(envelope->messageData+i, bufLen-i, c);
        i += write_string(envelope->messageData+i, bufLen-i, "\n");
        envelope->messageData[i++] = '\0';
        send_message(CRT_PID, envelope);
        envelope = NULL;
        release_processor();
    }
}

void funProcess(void) {
    int i;
    Envelope *sleepEnv = (Envelope *)request_memory_block();
    while (1) {
        for (i = 0; i < 5; ++i) {
            Envelope *envelope = (Envelope *)request_memory_block();
            uint8_t index = 0;
            uint8_t bufLen = MESSAGEDATA_SIZE_BYTES - 1; // -1 for null byte

            index += write_string(envelope->messageData+index, bufLen-index, "Fun ");
            index += write_uint32(envelope->messageData+index, bufLen-index, i, 1);
            index += write_string(envelope->messageData+index, bufLen-index, "\n");
            envelope->messageData[index++] = '\0';
            send_message(CRT_PID, envelope);
            envelope = NULL;
        }
        sleep(3000, NULL, sleepEnv);
    }
}

void schizophrenicProcess(void) {
    int i;
    Envelope *sleepEnv = (Envelope *)request_memory_block();
    while (1) {
        for (i = 9; i >= 5; --i) {
            Envelope *envelope = (Envelope *)request_memory_block();
            uint8_t index = 0;
            uint8_t bufLen = MESSAGEDATA_SIZE_BYTES - 1; // -1 for null byte

            index += write_string(envelope->messageData+index, bufLen-index, "Schizophrenic ");
            index += write_uint32(envelope->messageData+index, bufLen-index, i, 1);
            index += write_string(envelope->messageData+index, bufLen-index, "\n");
            envelope->messageData[index++] = '\0';
            send_message(CRT_PID, envelope);
            envelope = NULL;
        }
        sleep(4000, NULL, sleepEnv);
    }
}

void fibProcess(void) {
    uint32_t temp;
    uint32_t cur;
    uint32_t prev;
    uint32_t idx;
    Envelope *envelope = NULL;
    Envelope *sleepEnv = (Envelope *)request_memory_block();

    while (1) {
        prev = 1;
        cur = 1;
        idx = 0;
        while (cur < 1000000000) {
            uint8_t index = 0;
            uint8_t bufLen = MESSAGEDATA_SIZE_BYTES - 1; // -1 for null byte
            temp = prev;
            prev = cur;
            cur = cur + temp;
            idx++;

            envelope = (Envelope *)request_memory_block();
            index += write_string(envelope->messageData+index, bufLen-index, "fib(");
            index += write_uint32(envelope->messageData+index, bufLen-index, idx, 1);
            index += write_string(envelope->messageData+index, bufLen-index, ") = ");
            index += write_uint32(envelope->messageData+index, bufLen-index, cur, 1);
            index += write_string(envelope->messageData+index, bufLen-index, "\n");
            envelope->messageData[index++] = '\0';
            send_message(CRT_PID, envelope);
            envelope = NULL;

            if (idx % 5 == 0) {
                sleep(1000, NULL, sleepEnv);
            }
        }
    }
}

typedef struct mmNode mmNode;
struct mmNode {
    mmNode *next;
};

void memoryMuncherProcess(void) {
    mmNode *memList = NULL;
    void *tempBlock = NULL;
    mmNode *tempNode = NULL;
    Envelope *envelope = NULL;
    uint8_t index = 0;

    while (1) {
        uint8_t bufLen = 0;
        tempBlock = try_request_memory_block();
        if (tempBlock == NULL) {
            break;
        }

        tempNode = (mmNode *)tempBlock;
        tempNode->next = memList;
        memList = tempNode;
        tempNode = NULL;

        envelope = (Envelope *)try_request_memory_block();
        if (envelope == NULL) {
            break;
        }

        index = 0;
        bufLen = MESSAGEDATA_SIZE_BYTES - 1; // -1 for null byte
        index += write_string(envelope->messageData+index, bufLen-index, "I have eaten ");
        index += write_uint32(envelope->messageData+index, bufLen-index, (uint32_t)memList, 0);
        index += write_string(envelope->messageData+index, bufLen-index, ".\n");
        envelope->messageData[index++] = '\0';
        send_message(CRT_PID, envelope);
        envelope = NULL;
    }

    index = 0;
    envelope = (Envelope *)request_memory_block();
    index += write_string(envelope->messageData+index, 28, "I am out of things to eat.\n");
    envelope->messageData[index++] = '\0';
    send_message(CRT_PID, envelope);
    envelope = NULL;

    release_memory_block(request_memory_block()); // Should block

    index = 0;
    envelope = (Envelope *)request_memory_block();
    index += write_string(envelope->messageData+index, MESSAGEDATA_SIZE_BYTES-index, "I am too full.  I will release all the memory that I ate.\n");
    envelope->messageData[index++] = '\0';
    send_message(CRT_PID, envelope);
    envelope = NULL;
    while (memList != NULL) {
        tempNode = memList->next;
        release_memory_block((void *)memList);
        memList = tempNode;
    }

    set_process_priority(4, get_process_priority(1)); // funProcess pid = 1

    while (1) {
        envelope = receive_message(NULL);
        release_memory_block((void *)envelope);
    }
}

void releaseProcess(void) {
    void *mem = request_memory_block();
    Envelope *envelope = (Envelope *)request_memory_block();
    uint8_t index = 0;
    char *buf = envelope->messageData;
    size_t bufLen = MESSAGEDATA_SIZE_BYTES-1; // -1 for \0

    index += write_string(buf+index, bufLen-index, "releaseProcess: taken mem ");
    index += write_uint32(buf+index, bufLen-index, (uint32_t)mem, 0);
    index += write_string(buf+index, bufLen-index, "\n");
    buf[index++] = '\0';
    buf = NULL;
    send_message(CRT_PID, envelope);
    envelope = NULL;

    set_process_priority(pid(), get_process_priority(1));
    release_processor();

    envelope = (Envelope *)request_memory_block();
    index = 0;
    bufLen = MESSAGEDATA_SIZE_BYTES-1;
    buf = envelope->messageData;

    index += write_string(buf+index, bufLen-index, "releaseProcess: I am in control\n");
    buf[index++] = '\0';
    buf = NULL;
    send_message(CRT_PID, envelope);
    envelope = NULL;
    release_memory_block(mem);

    set_process_priority(4, 3);
    envelope = receive_message(NULL);
}

// Clock-related

typedef enum ClockCmdType ClockCmdType;
enum ClockCmdType {
    PRINT_TIME,
    RESET_TIME,
    SET_TIME,
    TERMINATE,
};

typedef struct ClockCmd ClockCmd;
struct ClockCmd {
    ClockCmdType cmdType;

    uint32_t currentTime;
    int32_t offset;
    uint32_t isRunning;

    Envelope *selfEnvelope;
    Envelope *receivedEnvelope;
};

void initClockCommand(ClockCmd *command) {
    command->cmdType = TERMINATE;

    command->currentTime = 0;
    command->offset = 0;
    command->isRunning = 0;

    command->selfEnvelope = (Envelope *)request_memory_block();
    command->receivedEnvelope = NULL;
}

uint8_t parseTime(char *message, int32_t *offset) {
    uint32_t field = 0;
    uint32_t requestedTime = 0;
    size_t n = 0;

    // TODO(sanjay): consider writing parseTime in the same style as all the
    // string handling APIs for consistency. That is to say:
    // size_t read_time(char *buf, size_t bufLen, uint32_t *time)

    // Check for any invalid characters.
    if (message[0] != '%' ||
            message[1] != 'W' ||
            message[2] != 'S' ||
            message[3] != ' ' ||
            message[6] != ':' ||
            message[9] != ':' ||
            message[10] < '0' || message[10] > '9' ||
            message[11] < '0' || message[11] > '9') {
        return EINVAL;
    }

    // Read hours field.
    n = read_uint32(message+4, 2, &field);
    if (field > 23 || n < 2) {
        return EINVAL;
    }

    requestedTime += (field * SECONDS_IN_HOUR * MILLISECONDS_IN_SECOND);

    // Read minutes field.
    n = read_uint32(message+7, 2, &field);
    if (field > 59 || n < 2) {
        return EINVAL;
    }

    requestedTime += (field * SECONDS_IN_MINUTE * MILLISECONDS_IN_SECOND);

    // Read seconds field.
    n = read_uint32(message+10, 2, &field);
    if (field > 59 || n < 2) {
        return EINVAL;
    }

    requestedTime += (field * MILLISECONDS_IN_SECOND);

    *offset = requestedTime - get_time();

    return SUCCESS;
}


void parseClockMessage(ClockCmd *command) {
    // Note:  when setting the command type to PRINT_TIME,
    // it will set isRunning to true and send a delayed message
    // to itself.
    uint8_t status = 0;
    Envelope *envelope = command->receivedEnvelope;

    if (envelope->srcPid == CLOCK_PID) {
        if (command->isRunning) {
            command->cmdType = PRINT_TIME;
        }
        return;
    } else if (envelope->srcPid != KEYBOARD_PID) {
        release_memory_block(envelope);
        return;
    }

    switch (envelope->messageData[2]) {
        case 'R':
            command->cmdType = RESET_TIME;
            break;
        case 'T':
            command->cmdType = TERMINATE;
            break;
        case 'S':
            command->cmdType = SET_TIME;
            break;
        default:
            return;
    }

    switch(command->cmdType) {
        case RESET_TIME:
            command->offset = -1 * (int32_t)command->currentTime;
            if (command->isRunning == 0) {
                command->cmdType = PRINT_TIME;
            }
            break;
        case SET_TIME:
            status = parseTime(envelope->messageData, &(command->offset));
            if (status == SUCCESS && command->isRunning == 0) {
                command->cmdType = PRINT_TIME;
            }
            break;
        case TERMINATE:
            command->isRunning = 0;
            break;
        default:
            break;
    }

    release_memory_block(envelope);
}

void printTime(uint32_t currentTime, uint32_t offset) {
    uint32_t clockTime = 0;
    uint32_t field = 0;
    uint32_t index = 0;
    Envelope *printMessage = (Envelope *)request_memory_block();
    char *buf = printMessage->messageData;
    size_t bufLen = MESSAGEDATA_SIZE_BYTES-1;

    clockTime = (currentTime + offset) % (SECONDS_IN_DAY * MILLISECONDS_IN_SECOND);
    clockTime /= MILLISECONDS_IN_SECOND;

    // Add ANSI colour code.
    buf[index++] = FC_RED + (clockTime % 6);

    // Print hours.
    field = clockTime / SECONDS_IN_HOUR;
    clockTime %= SECONDS_IN_HOUR;
    index += write_uint32(buf+index, bufLen-index, field, 2);

    buf[index++] = ':';

    // Print minutes.
    field = clockTime / SECONDS_IN_MINUTE;
    clockTime %= SECONDS_IN_MINUTE;
    index += write_uint32(buf+index, bufLen-index, field, 2);

    buf[index++] = ':';

    // Print seconds.
    field = clockTime;
    index += write_uint32(buf+index, bufLen-index, field, 2);

    index += write_string(buf+index, bufLen-index, "\n");
    buf[index++] = '\0';
    send_message(CRT_PID, printMessage);
}

void clockProcess(void) {
    ClockCmd command;
    Envelope *envelope = NULL;

    initClockCommand(&command);

    envelope = (Envelope *)request_memory_block();
    envelope->messageData[0] = 'w';
    // TODO (alex) - make message type matter
    send_message(KEYBOARD_PID, envelope);
    envelope = NULL;

    while (1) {
        command.receivedEnvelope = receive_message(NULL);
        command.currentTime = get_time();

        parseClockMessage(&command);

        if (command.cmdType == PRINT_TIME) {
            printTime(command.currentTime, command.offset);
            command.isRunning = 1;
            delayed_send(CLOCK_PID, command.selfEnvelope, 1000);
        }
    }
}

uint8_t processSetMessage(char *message) {
    uint8_t index = 2;
    uint8_t numLength = 0;
    uint32_t pid = 0;
    uint32_t priority = 0;

    if (message[index++] != ' ') {
        return EINVAL;
    }

    // Starts at index 3 (after '%C ')
    numLength = read_uint32(message + index, MESSAGEDATA_SIZE_BYTES - index, &pid);

    if (numLength == 0) {
        return EINVAL;
    }

    index += numLength;

    if (message[index++] != ' ') {
        return EINVAL;
    }

    numLength = read_uint32(message + index, MESSAGEDATA_SIZE_BYTES - index, &priority);

    if (numLength == 0 || priority > 255) {
        return EINVAL;
    }

    return set_process_priority(pid, priority);

}

void printSetErrorMessage(Envelope *envelope) {
    uint8_t index = 0;
    char *messageData = envelope->messageData;

    index += write_string(messageData + index, MESSAGEDATA_SIZE_BYTES - 1, "Please provide a proper process ID and priority.\n");
    messageData[index++] = '\0';
    send_message(CRT_PID, envelope);
}

void setPriorityProcess(void) {
    Envelope *envelope = NULL;
    char *message = NULL;
    uint8_t status = 0;

    envelope = (Envelope *)request_memory_block();
    envelope->dstPid = KEYBOARD_PID;
    envelope->messageData[0] = 'c';
    send_message(KEYBOARD_PID, envelope);
    envelope = NULL;

    while(1) {
        envelope = receive_message(NULL);

        if (envelope->srcPid != KEYBOARD_PID) {
            release_memory_block((void *)envelope);
            continue;
        }

        message = envelope->messageData;

        // Check for an invalid message type.
        if (message[0] != '%' || message[1] != 'C') {
            release_memory_block((void *)envelope);
            continue;
        }

        status = processSetMessage(message);

        if (status == EINVAL) {
            printSetErrorMessage(envelope);
        } else {
            release_memory_block((void *)envelope);
        }
    }
}

void stressAProcess(void) {
    Envelope *env = (Envelope *)request_memory_block();
    uint32_t num = 0;

    env->messageData[0] = 'Z';
    send_message(KEYBOARD_PID, env);
    env = NULL;

    //TODO(shale): their pseudocode says to loop until we receive a message from
    //             the keyboard command, but here we assume we only receive from
    //             the keyboard command. Verify assumption is reasonable once
    //             done creating the stress tests.
    env = receive_message(NULL);
    release_memory_block(env);
    env = NULL;

    while (1) {
        env = (Envelope *)request_memory_block();
        env->messageType = MT_COUNT_REPORT;

        num++;
        env->messageData[0] = (uint8_t)(num >> (8 * 3));
        env->messageData[1] = (uint8_t)(num >> (8 * 2));
        env->messageData[2] = (uint8_t)(num >> (8 * 1));
        env->messageData[3] = (uint8_t)(num >> (8 * 0));

        send_message(STRESS_B_PID, env);
        env = NULL;
        release_processor();
    }
}

void stressBProcess(void) {
    Envelope *envelope = NULL;
    while (1) {
        envelope = receive_message(NULL);
        if (envelope->srcPid == STRESS_A_PID) {
            send_message(STRESS_C_PID, envelope);
        } else {
            release_memory_block((void *) envelope);
        }
        envelope = NULL;
    }
}

void stressCProcess(void) {
    Envelope *msg = NULL;
    Envelope *msgQueue = NULL;
    Envelope *sleepEnv = (Envelope *)request_memory_block();

    while (1) {
        // NOTE(shale): we deviate from the spec here. We deal with the msgQueue
        // inside the sleep function, not here.
        if (msgQueue != NULL) {
            msg = msgQueue;
            msgQueue = msgQueue->next;
        } else {
            msg = receive_message(NULL);
        }
        if (msg->messageType == MT_COUNT_REPORT) {
            // TODO(shale): determine if we want to filter in other locations as well.
            uint32_t happyNumber =
                (((uint32_t) msg->messageData[0]) << (8*3)) +
                (((uint32_t) msg->messageData[1]) << (8*2)) +
                (((uint32_t) msg->messageData[2]) << (8*1)) +
                (((uint32_t) msg->messageData[3]) << (8*0));

            if (happyNumber % 20 == 0) {
                uint8_t i = 0;
                size_t bufLen = MESSAGEDATA_SIZE_BYTES - 1; // -1 for null byte
                i += write_string(msg->messageData+i, bufLen-i, "C Proc: ");
                i += write_uint32(msg->messageData+i, bufLen-i, happyNumber, 0);
                i += write_string(msg->messageData+i, bufLen-i, "\n");
                msg->messageData[i++] = '\0';
                send_message(CRT_PID, msg);
                msg = NULL;

                // TODO (all): fix deadlock
                sleep(10 * 1000, &msgQueue, sleepEnv);
            }
        }
        if (msg != NULL) {
            release_memory_block((void *)msg);
            msg = NULL;
        }
        release_processor();
    }
}
