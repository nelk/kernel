#ifndef HELPERS_H
#define HELPERS_H

#include "common_types.h"

uint32_t read_uint32(char *buffer, uint8_t length);
uint8_t write_ansi_escape(char *buffer, uint8_t num);
uint32_t write_string(char *buffer, char *msg, uint8_t maxLength);
uint8_t write_uint32(char *buffer, uint32_t number, uint8_t minDigits);

#endif
