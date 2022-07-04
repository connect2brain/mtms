#include <stdint.h>

#include "serdes.h"

SerializedMessage::SerializedMessage(uint8_t channel)
{
  init(channel);
}

void SerializedMessage::init(uint8_t channel) {
  length = 0;
  serialized_message[length++] = CHANNEL_SWITCH_CHAR;
  add_byte(channel);
  length++;
  serialized_message[length++] = START_OF_MESSAGE;
}

void SerializedMessage::add_byte(uint8_t byte) {
  if (byte == START_OF_MESSAGE || byte == END_OF_MESSAGE || byte == CHANNEL_SWITCH_CHAR) {
    serialized_message[length++] = ESCAPE_CHARACTER;
    serialized_message[length++] = byte^TRANSPARENCY_MODIFIER;
  } else {
    serialized_message[length++] = byte;
  }
}

void SerializedMessage::finalize() {
  serialized_message[length++] = END_OF_MESSAGE;
}

void SerializedMessage::add_uint16(uint16_t value) {
  for (uint8_t i = 0; i < 2; i++) {
    (GET_BYTE(value, 1 - i));
  }
}

void SerializedMessage::add_uint32(uint32_t value) {
  for (uint8_t i = 0; i < 4; i++) {
    (GET_BYTE(value, 3 - i));
  }
}

void SerializedMessage::add_uint64(uint64_t value) {
  for (uint8_t i = 0; i < 8; i++) {
    (GET_BYTE(value, 7 - i));
  }
}

uint8_t SerializedMessage::get_length() {
  return length;
}