#ifndef HELPERS_H
#define HELPERS_H

#include "common_types.h"

size_t memset(uint8_t *buf, size_t bufLen, uint8_t val);

size_t read_uint32(char *buf, size_t bufLen, uint32_t *out);
size_t write_ansi_escape(char *buf, size_t bufLen, uint8_t num);
size_t write_string(char *buf, size_t bufLen, char *msg);
size_t write_uint32(
    char *buf,
    size_t bufLen,
    uint32_t number,
    uint8_t minDigits
);
void copy_envelope(Envelope *dst, Envelope *src);


#endif
