#include <stddef.h>

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
        uart_put_string(UART_NUM, c);
        release_processor();
    }
}

void funProcess(void) {
    int i;
    while (1) {
        for (i = 0; i < 5; ++i) {
            uart_put_string(UART_NUM, "Fun ");
            uart_put_char(UART_NUM, i + '0');
            uart_put_string(UART_NUM, "\n\r");
        }
        release_processor();
    }
}

void schizophrenicProcess(void) {
    int i;
    while (1) {
        for (i = 9; i >= 5; --i) {
            uart_put_string(UART_NUM, "Schizophrenic ");
            uart_put_char(UART_NUM, i + '0');
            uart_put_string(UART_NUM, "\n\r");
        }
        release_processor();
    }
}

void print_uint32(uint32_t i) {
    int base = 1;

    if (i == 0) {
        uart_put_char(UART_NUM, '0');
        return;
    }

    while (i % base != i) {
        base *= 10;
    }
    base /= 10;

    while (base > 0) {
        uart_put_char(UART_NUM, (i/base) + '0');
        i %= base;
        base /= 10;
    }

}

void fibProcess(void) {
    uint8_t temp;
    uint8_t cur;
    uint8_t prev;
    uint8_t idx;

    while (1) {
        prev = 1;
        cur = 1;
        idx = 0;
        while (cur < 1000000000) {
            temp = prev;
            prev = cur;
            cur = cur + temp;
            idx++;

            uart_put_string(UART_NUM, "fib(");
            print_uint32(idx);
            uart_put_string(UART_NUM, ") = ");
            print_uint32(cur);
            uart_put_string(UART_NUM, "\r\n");

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

        uart_put_string(UART_NUM, "I have eaten ");
        print_uint32((uint32_t)memList);
        uart_put_string(UART_NUM, ".\r\n");
    }

    uart_put_string(UART_NUM, "I am out of things to eat.\r\n");

    release_memory_block(request_memory_block()); // Should block

    uart_put_string(UART_NUM, "I'm too full, I will release all the memory that I ate.\r\n");
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
    uart_put_string(UART_NUM, "releaseProcess: taken mem ");
    print_uint32((uint32_t)mem);
    uart_put_string(UART_NUM, "\r\n");

    set_process_priority(5, get_process_priority(1)); // funProcess pid = 1
    release_processor();

    uart_put_string(UART_NUM, "releaseProcess: I am in control\r\n");
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
    uint32_t offset;
    uint32_t isRunning;

    ProcId myPid;

    Envelope *selfEnvelope;
    Envelope *receivedEnvelope;
};

void initClockCommand(ClockCmd *command) {
    command->cmdType = TERMINATE;

    command->currentTime = 0;
    command->offset = 0;
    command->isRunning = 0;

    command->myPid = 0; // TODO: Maybe add user API to get own PID.

    command->selfEnvelope = (Envelope *)request_memory_block();
    command->receivedEnvelope = NULL;
}

void parseClockMessage(ClockCmd *command) {
    uint8_t status = 0;
    if (envelope->srcPid == myPid) {
        command->cmdType = PRINT_TIME;
        return;
    }

    // TODO: Parse envelope message, set proper command type.

    switch(command->cmdType) {
        case RESET_TIME:
            command->offset = command->currentTime;
            command->isRunning = 1;
            command->cmdType = PRINT_TIME;
            break;
        case SET_TIME:
            status = parseTime(command->receivedEnvelope->messageData, command->offset);
            if (status == 0) {
                command->isRunning = 1;
                command->cmdType = PRINT_TIME;
            } else {
                uart_put_string(UART_NUM, "Please give input in the form \"%WS hh:mm:ss\" with valid values.");
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

uint8_t parseTime(char[] message, uint32_t *offset) {
    // TODO: Parse message.  Give error code if formatting is incorrect
    // or values are out of bounds.  Also, use enums/generic defines for codes.
    return 0;
}

void printTime(uint32_t currentTime, uint32_t offset) {
    uint32_t clockTime = 0;

    clockTime = (currentTime + offset) % (SECONDS_IN_DAY * MILLISECONDS_IN_SECOND);
    clockTime /= MILLISECONDS_IN_SECOND;

    // Print hours.
    print_uint32(clockTime / SECONDS_IN_HOUR);
    clockTime %= SECONDS_IN_HOUR;

    uart_put_string(UART_NUM, ":");

    // Print minutes.
    print_uint32(clockTime / SECONDS_IN_MINUTE);
    clockTime %= SECONDS_IN_MINUTE;

    uart_put_string(UART_NUM, ":");

    // Print seconds.
    print_uint32(clockTime);
    uart_put_string(UART_NUM, "\r\n");
}

void clockProcess(void) {
    ClockCmd command;

    initClockCommand(&command);

    // TODO: Register commands %WR, %WS hh:mm:ss, and %WT with keyboard decoder.

    while (1) {
        command->receivedEnvelope = receive_message(NULL);
        command->currentTime = getTime();

        parseClockMessage(receivedEnvelope, &command);

        if (command->cmdType == PRINT_TIME) {
            printTime(currentTime, offset);
            delayed_send(myPid, selfEnvelope, 1000);
        }
    }
}
