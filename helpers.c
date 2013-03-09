#include "helpers.h"

void write_uint32(uint32_t number, char *buffer, uint32_t *startIndex) {
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

