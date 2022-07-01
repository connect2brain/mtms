#include <stdint.h>

#include "serdes.h"

#define GET_BYTE(var,n) (uint8_t)((var) >> (8 * (n)))

uint8_t serialized_message[MAX_SERIALIZED_MESSAGE_LENGTH] = {0};
uint8_t length = 0;

#define ESCAPE_CHARACTER 0x1B
#define TRANSPARENCY_MODIFIER 0x20
#define START_OF_MESSAGE 0xFE
#define END_OF_MESSAGE 0xFF
#define CHANNEL_SWITCH_CHAR 0xFD

void init_serialized_message(uint8_t channel) {
  length = 0;
  serialized_message[length++] = CHANNEL_SWITCH_CHAR;
  add_byte_to_serialized_message(channel);
  length++;
  serialized_message[length++] = START_OF_MESSAGE;
}

void finalize_serialized_message() {
  serialized_message[length++] = END_OF_MESSAGE;
}

void add_byte_to_serialized_message(uint8_t byte) {
  if (byte == START_OF_MESSAGE || byte == END_OF_MESSAGE || byte == CHANNEL_SWITCH_CHAR) {
    serialized_message[length++] = ESCAPE_CHARACTER;
    serialized_message[length++] = byte^TRANSPARENCY_MODIFIER;
  } else {
    serialized_message[length++] = byte;
  }
}

void add_uint16_to_serialized_message(uint16_t value) {
  for (uint8_t i = 0; i < 2; i++) {
    add_byte_to_serialized_message(GET_BYTE(value, 1 - i));
  }
}

void add_uint32_to_serialized_message(uint32_t value) {
  for (uint8_t i = 0; i < 4; i++) {
    add_byte_to_serialized_message(GET_BYTE(value, 3 - i));
  }
}

void add_uint64_to_serialized_message(uint64_t value) {
  for (uint8_t i = 0; i < 8; i++) {
    add_byte_to_serialized_message(GET_BYTE(value, 7 - i));
  }
}
