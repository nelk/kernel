#include "helpers.h"

size_t write_mem(uint8_t *buf, size_t bufLen, uint8_t val) {
    size_t temp = bufLen;

    if (buf == NULL) {
        return bufLen;
    }

    while(bufLen > 0) {
        *(buf++) = val;
        --bufLen;
    }

    return temp;
}

uint8_t is_printable(uint8_t c) {
    return c >= ' ' && c <= '~';
}

uint8_t is_numeric(uint8_t c) {
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

    // BUG(sanjay): this implementation allows overflow of a uint32.
    // read_uint32("5000000000", 96, ptr) will return 10, and set ptr
    // to 705032705 (this is hardware dependant). It should return 9,
    // and set ptr to 500000000. On the other hand,
    // read_uint32("4294967295", 96, ptr) should return 10, and set ptr to
    // UINT_MAX, so we cannot just stop at 9 bytes.
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

void copy_envelope(Envelope *dst, Envelope *src) {
    size_t idx = 0;
    if (dst == NULL || src == NULL) {
        return;
    }
    dst->messageType = src->messageType;
    for (idx = 0; idx < MESSAGEDATA_SIZE_BYTES; ++idx) {
        dst->messageData[idx] = src->messageData[idx];
    }
}
