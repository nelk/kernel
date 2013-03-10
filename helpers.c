#include "helpers.h"

uint32_t read_uint32(char *buffer, uint8_t length) {
    uint32_t number = 0;
    uint32_t i = 0;

    for(; i < length; ++i) {
        number *= 10;
        number += (uint32_t)(buffer[i] - '0');
    }

    return number;
}

uint8_t write_uint32(char *buffer, uint32_t number, uint8_t minDigits) {
    uint32_t tempNumber = number;
    uint8_t numDigits = 0;

    while (tempNumber > 0) {
        ++numDigits;
        tempNumber /= 10;
    }

    if (minDigits < 1) {
        minDigits = 1;
    }

    if (numDigits < minDigits) {
        numDigits = minDigits;
    }

    tempNumber = numDigits;

    while (numDigits > 0) {
        buffer[numDigits-1] = (char)((number % 10)+'0');
        number /= 10;
        --numDigits;
    }

    return (uint8_t)tempNumber;
}

uint32_t write_string(char *buffer, char *msg, uint8_t maxLength) {
	uint8_t written = 0;
	while (*msg != '\0' && maxLength > 0) {
		*(buffer++) = *(msg++);
		--maxLength;
		++written;
	}
	return written;
}

uint8_t write_ansi_escape(char *buffer, uint8_t num) {
    uint8_t idx = 0;
    buffer[idx++] = '\x1b';
    buffer[idx++] = '[';
    idx += write_uint32(buffer+idx, num, 0);
    buffer[idx++] = 'm';
    return idx;
}

