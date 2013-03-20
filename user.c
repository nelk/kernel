#include <stddef.h>

#include "rtx.h"
#include "uart_polling.h"
#include "user.h"
#include "helpers.h"

// User Land

void sleep(uint32_t ms) {
    // TODO(alex): make this more robust by forwarding non-sleep messages to ourselves on a delay

    Envelope* env = (Envelope *)request_memory_block();
    delayed_send(pid(), env, ms);
    env = receive_message(NULL);
    release_memory_block((void*)env);
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

        i += write_string(envelope->messageData+i, c, 100);
        i += write_string(envelope->messageData+i, "\r\n", 2);
        envelope->messageData[i++] = '\0';
        send_message(CRT_PID, envelope);
        envelope = NULL;
        release_processor();
    }
}

void funProcess(void) {
    int i;
    while (1) {
        for (i = 0; i < 5; ++i) {
            Envelope *envelope = (Envelope *)request_memory_block();
            uint8_t index = 0;

            index += write_string(envelope->messageData+index, "Fun ", 4);
            index += write_uint32(envelope->messageData+index, i, 1);
            index += write_string(envelope->messageData+index, "\r\n", 2);
            envelope->messageData[index++] = '\0';
            send_message(CRT_PID, envelope);
            envelope = NULL;
        }
        sleep(3000);
    }
}

void schizophrenicProcess(void) {
    int i;
    while (1) {
        for (i = 9; i >= 5; --i) {
            Envelope *envelope = (Envelope *)request_memory_block();
            uint8_t index = 0;

            index += write_string(envelope->messageData+index, "Schizophrenic ", 14);
            index += write_uint32(envelope->messageData+index, i, 1);
            index += write_string(envelope->messageData+index, "\r\n", 2);
            envelope->messageData[index++] = '\0';
            send_message(CRT_PID, envelope);
            envelope = NULL;
        }
        sleep(4000);
    }
}

void fibProcess(void) {
    uint32_t temp;
    uint32_t cur;
    uint32_t prev;
    uint32_t idx;
    Envelope *envelope = NULL;

    while (1) {
        prev = 1;
        cur = 1;
        idx = 0;
        while (cur < 1000000000) {
            uint8_t index = 0;
            temp = prev;
            prev = cur;
            cur = cur + temp;
            idx++;

            envelope = (Envelope *)request_memory_block();
            index += write_string(envelope->messageData+index, "fib(", 4);
            index += write_uint32(envelope->messageData+index, idx, 1);
            index += write_string(envelope->messageData+index, ") = ", 4);
            index += write_uint32(envelope->messageData+index, cur, 1);
            index += write_string(envelope->messageData+index, "\r\n", 2);
            envelope->messageData[index++] = '\0';
            send_message(CRT_PID, envelope);
            envelope = NULL;

            if (idx % 5 == 0) {
                sleep(1000);
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
        index = 0;
        tempBlock = try_request_memory_block();
        if (tempBlock == NULL) {
            break;
        }

        tempNode = (mmNode *)tempBlock;
        tempNode->next = memList;
        memList = tempNode;
        tempNode = NULL;

        envelope = (Envelope *)request_memory_block();
        index += write_string(envelope->messageData+index, "I have eaten ", 13);
        index += write_uint32(envelope->messageData+index, (uint32_t)memList, 0);
        index += write_string(envelope->messageData+index, ".\r\n", 3);
        envelope->messageData[index++] = '\0';
        send_message(CRT_PID, envelope);
        envelope = NULL;
    }

    index = 0;
    envelope = (Envelope *)request_memory_block();
    index += write_string(envelope->messageData+index, "I am out of things to eat.\r\n", 28);
    envelope->messageData[index++] = '\0';
    send_message(CRT_PID, envelope);
    envelope = NULL;

    release_memory_block(request_memory_block()); // Should block

    index = 0;
    envelope = (Envelope *)request_memory_block();
    index += write_string(envelope->messageData+index, "I am too full.  I will release all the memory that I ate.\r\n", 59);
    envelope->messageData[index++] = '\0';
    send_message(CRT_PID, envelope);
    envelope = NULL;
    while (memList != NULL) {
        tempNode = memList->next;
        release_memory_block((void *)memList);
        memList = tempNode;
    }

    set_process_priority(4, get_process_priority(1)); // funProcess pid = 1

    envelope = receive_message(NULL);
}

void releaseProcess(void) {
    void *mem = request_memory_block();
    Envelope *envelope = (Envelope *)request_memory_block();
    uint8_t index = 0;

    index += write_string(envelope->messageData+index, "releaseProcess: taken mem ", 26);
    index += write_uint32(envelope->messageData+index, (uint32_t)mem, 0);
    index += write_string(envelope->messageData+index, "\r\n", 2);
    envelope->messageData[index++] = '\0';
    send_message(CRT_PID, envelope);
    envelope = NULL;

    set_process_priority(pid(), get_process_priority(1));
    release_processor();

    index = 0;
    envelope = (Envelope *)request_memory_block();
    index += write_string(envelope->messageData+index, "releaseProcess: I am in control\r\n", 32);
    envelope->messageData[index++] = '\0';
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

    // Check for any invalid characters.
    if (message[0] != '%' ||
            message[1] != 'W' ||
            message[2] != 'S' ||
            message[3] != ' ' ||
            message[4] < '0' || message[4] > '9' ||
            message[5] < '0' || message[5] > '9' ||
            message[6] != ':' ||
            message[7] < '0' || message[7] > '9' ||
            message[8] < '0' || message[8] > '9' ||
            message[9] != ':' ||
            message[10] < '0' || message[10] > '9' ||
            message[11] < '0' || message[11] > '9') {
        return EINVAL;
    }

    // Read hours field.
    field = read_uint32(message+4, 2);

    if (field > 23) {
        return EINVAL;
    }

    requestedTime += (field * SECONDS_IN_HOUR * MILLISECONDS_IN_SECOND);

    // Read minutes field.
    field = read_uint32(message+7, 2);

    if (field > 59) {
        return EINVAL;
    }

    requestedTime += (field * SECONDS_IN_MINUTE * MILLISECONDS_IN_SECOND);

    // Read seconds field.
    field = read_uint32(message+10, 2);

    if (field > 59) {
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
    char *messageData = printMessage->messageData;

    clockTime = (currentTime + offset) % (SECONDS_IN_DAY * MILLISECONDS_IN_SECOND);
    clockTime /= MILLISECONDS_IN_SECOND;

    // Add ANSI colour code.
    index += write_ansi_escape(messageData+index, 31 + (clockTime % 6));

    // Print hours.
    field = clockTime / SECONDS_IN_HOUR;
    clockTime %= SECONDS_IN_HOUR;
    index += write_uint32(messageData + index, field, 2);

    messageData[index++] = ':';

    // Print minutes.
    field = clockTime / SECONDS_IN_MINUTE;
    clockTime %= SECONDS_IN_MINUTE;
    index += write_uint32(messageData + index, field, 2);

    messageData[index++] = ':';

    // Print seconds.
    field = clockTime;
    index += write_uint32(messageData + index, field, 2);

    // Add ANSI reset.
    index += write_ansi_escape(messageData+index, 0);

    index += write_string(messageData+index, "\r\n", 2);
    messageData[index++] = '\0';
    send_message(CRT_PID, printMessage);
}

void clockProcess(void) {
    ClockCmd command;
    Envelope *envelope = NULL;

    initClockCommand(&command);

    envelope = (Envelope *)request_memory_block();
    envelope->dstPid = KEYBOARD_PID;
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

uint8_t parseSetMessage(char *message) {
    uint8_t index = 2;
    uint8_t offsetIndex = 0;
    uint8_t pid = 0;
    uint8_t priority = 0;

    if (message[index++] != ' ') {
        return EINVAL;
    }

    // Starts at index 3 (after '%C ')
    offsetIndex = index;
    while (offsetIndex < MESSAGEDATA_SIZE_BYTES) {
        if (message[offsetIndex] == ' ') {
            break;
        }
        ++offsetIndex;
    }

    // Break if larger than two characters or less than one.
    if (offsetIndex == index + 1 || offsetIndex > index + 2) {
        return EINVAL;
    }

    // Break if the input is not an integer (up to two digits).
    if (message[index] < '0' || message[index] > '9' ||
        (index != offsetIndex - 1 && (message[index + 1] < '0' || message[index + 1] > '9'))) {
        return EINVAL;
    }

    pid = read_uint32(message + index, offsetIndex - index);

    ++offsetIndex;
    index = offsetIndex;

    // Starts after the second space.
    // TODO(Jon): Verify what the last character after a finished number would be.
    while (offsetIndex < MESSAGEDATA_SIZE_BYTES) {
        if (message[offsetIndex] == '\0') {
            break;
        }
        ++offsetIndex;
    }

    // Break if larger than three characters or less than one.
    if (offsetIndex == index + 1 || offsetIndex > index + 3) {
        return EINVAL;
    }

    // Break if the input is not an integer (up to three digits).
    if (message[index] < '0' || message[index] > '9' ||
        (index != offsetIndex - 2 && (message[index + 2] < '0' || message[index + 2] > '9')) ||
        (index != offsetIndex - 1 && (message[index + 1] < '0' || message[index + 1] > '9'))) {
        return EINVAL;
    }

    priority = read_uint32(message + index, offsetIndex - index);

    return set_process_priority(pid, priority);

}

void printSetErrorMessage(Envelope *envelope) {
    uint8_t index = 0;
    char *messageData = envelope->messageData;

    index += write_string(messageData + index, "Please provide a proper process ID and priority.\r\n", 50);
    messageData[index++] = '\0';
    send_message(CRT_PID, envelope);
}

void processSetMessage(void) {
    Envelope *envelope = NULL;
    char *message = NULL;
    uint8_t status = 0;

    envelope = receive_message(NULL);

    if (envelope->srcPid != KEYBOARD_PID) {
        release_memory_block((void *)envelope);
        return;
    }

    message = envelope->messageData;

    // Check for an invalid message type.
    if (message[0] != '%' || message[1] != 'C') {
        release_memory_block((void *)envelope);
        return;
    }

    status = parseSetMessage(message);

    if (status == EINVAL) {
        printSetErrorMessage(envelope);
    } else {
        release_memory_block((void *)envelope);
    }
}

void setPriorityProcess(void) {
    Envelope *envelope = NULL;

    envelope = (Envelope *)request_memory_block();
    envelope->dstPid = KEYBOARD_PID;
    envelope->messageData[0] = 'c';
    send_message(KEYBOARD_PID, envelope);
    envelope = NULL;

    while(1) {
        processSetMessage();
    }
}
