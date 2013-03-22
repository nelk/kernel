#include "helpers.h"

uint8_t is_numeric(char c) {
    return c >= '0' && c <= '9';
}

size_t read_uint32(char *buf, size_t bufLen, uint32_t *out) {
    uint32_t number = 0;
    size_t read = 0;

    if (buf == NULL) {
        return 0;
    }

    while (read < bufLen) {
        if (!is_numeric(*buf)) {
            break;
        }
        number *= 10;
        number += (uint32_t)(*buf - '0');
        read++;
        buf++;
    }

    if (out != NULL) {
        *out = number;
    }

    return read;
}

size_t write_uint32(
    char *buf,
    size_t bufLen,
    uint32_t number,
    uint8_t minDigits
) {
    // TODO(sanjay): this function might read better if we make it write
    // to a stack-allocated byte buffer of length digitsof(uint32_t)+1,
    // and then call write_string.

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
    if (numDigits > bufLen) {
        numDigits = bufLen;
    }

    tempNumber = numDigits;
    while (numDigits > 0) {
        if (buf != NULL) {
            buf[numDigits-1] = (char)((number % 10)+'0');
        }
        number /= 10;
        --numDigits;
    }

    return (uint8_t)tempNumber;
}

size_t write_string(char *buf, size_t bufLen, char *msg) {
    size_t written = 0;
    if (msg == NULL) {
        return 0;
    }
    while (*msg != '\0' && bufLen > 0) {
        if (buf != NULL) {
            *(buf++) = *msg;
        }

        ++msg;
        --bufLen;
        ++written;
    }
    return written;
}

size_t write_ansi_escape(char *buf, size_t bufLen, uint8_t num) {
    size_t idx = 0;
    idx += write_string(buf+idx, bufLen-idx, "\x1b[");
    idx += write_uint32(buf+idx, bufLen-idx, num, 0);
    idx += write_string(buf+idx, bufLen-idx, "m");
    return idx;
}

