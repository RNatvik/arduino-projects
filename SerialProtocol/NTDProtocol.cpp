#include "Arduino.h"
#include "NTDProtocol.h"

NTDProtocol::NTDProtocol() {

}

void NTDProtocol::write_header(int n, byte types[]) {
  Serial.write(0xff);
  Serial.write(n);
  for (int i = 0; i < n; i++) {
    Serial.write(types[i]);
  }
}

void NTDProtocol::write_data(bool input_value) {
  Serial.write(input_value);
}

void NTDProtocol::write_data(byte input_value) {
  Serial.write(input_value);
}

void NTDProtocol::write_data(int input_value) {
  union {
    byte bytes[2];
    int value;
  } UNION;
  UNION.value = input_value;
  Serial.write(UNION.bytes, 2);
}

void NTDProtocol::write_data(long input_value) {
  union {
    byte bytes[4];
    long value;
  } UNION;
  UNION.value = input_value;
  Serial.write(UNION.bytes, 4);
}


void NTDProtocol::write_data(float input_value) {
  union {
    byte bytes[4];
    float value;
  } UNION;
  UNION.value = input_value;
  Serial.write(UNION.bytes, 4);
}

void NTDProtocol::write_data(unsigned int input_value) {
  union {
    byte bytes[2];
    unsigned int value;
  } UNION;
  UNION.value = input_value;
  Serial.write(UNION.bytes, 2);
}

void NTDProtocol::write_data(unsigned long input_value) {
  union {
    byte bytes[4];
    unsigned long value;
  } UNION;
  UNION.value = input_value;
  Serial.write(UNION.bytes, 4);
}

void NTDProtocol::write_end() {
  Serial.write(0x00);
}

bool NTDProtocol::receive_data(byte data_buffer[]) {
  bool message_received = false;
  if (Serial.available() > 0) {
    bool start_received = false;
    while (!start_received) {
      start_received = (Serial.read() == 0xff);
    }
    int num = Serial.read();
    data_buffer[0] = num;
    byte type_buffer[num];
    message_received = fill_data(num, type_buffer, data_buffer);
  }
  return message_received;
}

bool NTDProtocol::fill_data(int n, byte type_buffer[], byte data_buffer[]) {
  bool success = true;
  for (int i = 0; i < n; i++) {
    type_buffer[i] = byte(Serial.read());
  }
  int index = 1;
  for (int i = 0; i < n; i++) {
    byte type = type_buffer[i];
    int number_of_bytes = 0;
    switch (type) {
      case NTDProtocol::_BOOL:
        number_of_bytes = 1;
        break;
      case NTDProtocol::_BYTE:
        number_of_bytes = 1;
        break;
      case NTDProtocol::_INT:
        number_of_bytes = 2;
        break;
      case NTDProtocol::_LONG:
        number_of_bytes = 4;
        break;
      case NTDProtocol::_FLOAT:
        number_of_bytes = 4;
        break;
      case NTDProtocol::_UINT:
        number_of_bytes = 2;
        break;
      case NTDProtocol::_ULONG:
        number_of_bytes = 4;
        break;
      default:
        success = false;
        break;

    }
    for (int j = index; j < index + number_of_bytes; j++) {
      data_buffer[j] = byte(Serial.read());
    }
    index += number_of_bytes;
  }
  data_buffer[index] = Serial.read();
  if (byte(data_buffer[index]) != 0x00) {
    success = false;
  }
  return success;
}


byte NTDProtocol::bool_code() {
  return _BOOL;
}

byte NTDProtocol::byte_code() {
  return _BYTE;
}

byte NTDProtocol::int_code() {
  return _INT;
}

byte NTDProtocol::long_code() {
  return _LONG;
}

byte NTDProtocol::float_code() {
  return _FLOAT;
}

byte NTDProtocol::uint_code() {
  return _UINT;
}

byte NTDProtocol::ulong_code() {
  return _ULONG;
}
