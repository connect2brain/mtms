#ifndef SERDES_H
#define SERDES_H

#include <stdint.h>

#define MAX_SERIALIZED_MESSAGE_LENGTH 100

void init_serialized_message(uint8_t channel);
void finalize_serialized_message();
void add_byte_to_serialized_message(uint8_t byte);
void add_uint16_to_serialized_message(uint16_t value);
void add_uint32_to_serialized_message(uint32_t value);
void add_uint64_to_serialized_message(uint64_t value);

extern uint8_t serialized_message[MAX_SERIALIZED_MESSAGE_LENGTH];
extern uint8_t length;

#endif
