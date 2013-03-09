#include <stddef.h>
#include <stdio.h>

#include "rtx.h"
#include "uart_polling.h"
#include "user.h"

// User Land

void nullProcess(void) {
    while (1) {
        release_processor();
    }
}


void printProcess(char *c) {
    while (1) {
        // uart_put_string(UART_NUM, c);
        release_processor();
    }
}

void funProcess(void) {
    int i;
    while (1) {
        for (i = 0; i < 5; ++i) {
            // uart_put_string(UART_NUM, "Fun ");
            // uart_put_char(UART_NUM, i + '0');
            // uart_put_string(UART_NUM, "\n\r");
        }
        release_processor();
    }
}

void schizophrenicProcess(void) {
    int i;
    while (1) {
        for (i = 9; i >= 5; --i) {
            // uart_put_string(UART_NUM, "Schizophrenic ");
            // uart_put_char(UART_NUM, i + '0');
            // uart_put_string(UART_NUM, "\n\r");
        }
        release_processor();
    }
}

void print_uint32(uint32_t i) {
    int base = 1;

    if (i == 0) {
        // uart_put_char(UART_NUM, '0');
        return;
    }

    while (i % base != i) {
        base *= 10;
    }
    base /= 10;

    while (base > 0) {
        // uart_put_char(UART_NUM, (i/base) + '0');
        i %= base;
        base /= 10;
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
            temp = prev;
            prev = cur;
            cur = cur + temp;
            idx++;

						envelope = (Envelope *)request_memory_block();
						// TODO: Replace with self-rolled method.
						sprintf(envelope->messageData, "fib(%d) = %d\r\n", idx, cur);
						send_message(CRT_PID, envelope);
						envelope = NULL;

            if (idx % 5 == 0) {
                release_processor();
            }
            //if (idx % 100 == 0) {
            //    set_process_priority(3, get_process_priority(1)); // funProcess pid = 1
            //}
        }

        release_processor();
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

    while (1) {
        tempBlock = try_request_memory_block();
        if (tempBlock == NULL) {
            break;
        }

        tempNode = (mmNode *)tempBlock;
        tempNode->next = memList;
        memList = tempNode;
        tempNode = NULL;

        // uart_put_string(UART_NUM, "I have eaten ");
        print_uint32((uint32_t)memList);
        // uart_put_string(UART_NUM, ".\r\n");
    }

    // uart_put_string(UART_NUM, "I am out of things to eat.\r\n");

    release_memory_block(request_memory_block()); // Should block

    // uart_put_string(UART_NUM, "I'm too full, I will release all the memory that I ate.\r\n");
    while (memList != NULL) {
        tempNode = memList->next;
        release_memory_block((void *)memList);
        memList = tempNode;
    }

    set_process_priority(4, get_process_priority(1)); // funProcess pid = 1

    printProcess("memoryMuncher\r\n");
}

void releaseProcess(void) {
    void *mem = request_memory_block();
    // uart_put_string(UART_NUM, "releaseProcess: taken mem ");
    print_uint32((uint32_t)mem);
    // uart_put_string(UART_NUM, "\r\n");

    set_process_priority(5, get_process_priority(1)); // funProcess pid = 1
    release_processor();

    // uart_put_string(UART_NUM, "releaseProcess: I am in control\r\n");
    release_memory_block(mem);

    //set_process_priority(4, get_process_priority(1)); // funProcess pid = 1
    printProcess("releaseProcess\r\n");
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

void write_uint32(uint32_t number, char *buffer, uint8_t *startIndex) {
    uint32_t tempNumber = number;
    uint8_t numDigits = 0;

    while (tempNumber > 0) {
        ++numDigits;
        tempNumber /= 10;
    }

    if (number < 10) {
        numDigits = 2;
    }

    buffer = buffer + *startIndex;
    *startIndex += numDigits;

    while (numDigits > 0) {
        buffer[numDigits-1] = (char)((number % 10)+'0');
        number /= 10;
        --numDigits;
    }
}

uint32_t get_uint32(char *buffer, uint8_t startIndex, uint8_t length) {
    uint32_t number = 0;
    uint32_t i = 0;

    for(; i < length; ++i) {
        number *= 10;
        number += (uint32_t)(buffer[i + startIndex] - '0');
    }

    return number;
}

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
    if (message[0] != '%' || message[1] != 'W' || message[2] != 'S' ||
        message[3] != ' ' || message[6] != ':' || message[9] != ':') {
        return EINVAL;
    }

    // Read hours field.
    if (message[4] < '0' || message[4] > '9' || message[5] < '0' || message[5] > '9') {
        return EINVAL;
    }

    field = get_uint32(message, 4, 2);

    if (field > 23) {
        return EINVAL;
    }

    requestedTime += (field * SECONDS_IN_HOUR * MILLISECONDS_IN_SECOND);

    // Read minutes field.
    if (message[7] < '0' || message[7] > '9' || message[8] < '0' || message[8] > '9') {
        return EINVAL;
    }

    field = get_uint32(message, 7, 2);

    if (field > 59) {
        return EINVAL;
    }

    requestedTime += (field * SECONDS_IN_MINUTE * MILLISECONDS_IN_SECOND);

    // Read seconds field.
    if (message[10] < '0' || message[10] > '9' || message[11] < '0' || message[11] > '9') {
        return EINVAL;
    }

    field = get_uint32(message, 10, 2);

    if (field > 59) {
        return EINVAL;
    }

    requestedTime += (field * MILLISECONDS_IN_SECOND);
		
    *offset = requestedTime - get_time();

    return SUCCESS;
}


void parseClockMessage(ClockCmd *command) {
    uint8_t status = 0;
    Envelope *envelope = command->receivedEnvelope;

    if (envelope == command->selfEnvelope) {
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
            command->cmdType = PRINT_TIME;
            break;
        case SET_TIME:
            status = parseTime(envelope->messageData, &(command->offset));
            if (status == SUCCESS) {
                command->cmdType = PRINT_TIME;
            } else {
                // uart_put_string(UART_NUM, "Please give input in the form \"%WS hh:mm:ss\" with valid values.");
                if (command->isRunning) {
                    command->cmdType = PRINT_TIME;
                }
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
    uint8_t index = 0;
    Envelope *printMessage = (Envelope *)request_memory_block();
    char *messageData = printMessage->messageData;

    clockTime = (currentTime + offset) % (SECONDS_IN_DAY * MILLISECONDS_IN_SECOND);
    clockTime /= MILLISECONDS_IN_SECOND;

    // Print hours.
    field = clockTime / SECONDS_IN_HOUR;
    clockTime %= SECONDS_IN_HOUR;
    write_uint32(field, messageData, &index);

    messageData[index++] = ':';

    // Print minutes.
    field = clockTime / SECONDS_IN_MINUTE;
    clockTime %= SECONDS_IN_MINUTE;
    write_uint32(field, messageData, &index);

    messageData[index++] = ':';

    // Print seconds.
    write_uint32(clockTime, messageData, &index);
    messageData[index++] = '\r';
    messageData[index++] = '\n';
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
