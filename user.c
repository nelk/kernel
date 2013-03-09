#include <stddef.h>

#include "proc.h"
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

            // uart_put_string(UART_NUM, "fib(");
            print_uint32(idx);
            // uart_put_string(UART_NUM, ") = ");
            print_uint32(cur);
            // uart_put_string(UART_NUM, "\r\n");

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
