#include "Arduino.h"
#include "RACProtocol.h"

RACProtocol::RACProtocol() {
}

RACProtocol::RACProtocol(int num_buffer, byte value_buffer[], int num_register, int register_size[], int index_array[]) {
  _VALUE_BUFFER = value_buffer;
  _REGISTER_SIZE = register_size;
  _INDEX_ARRAY = index_array;
  _NUM_REGISTERS = num_register;
  _NUM_BUFFER = num_buffer;
  int cumsum = 0;
  for (int i = 0; i < num_register; i++) {
    _INDEX_ARRAY[i] = cumsum;
    cumsum += _REGISTER_SIZE[i];
  }
  for (int i = 0; i < num_buffer; i++) {
    _VALUE_BUFFER[i] = 0;
  }
  primed = false;
  prime_amount = 0;
  primed_register = 0;
  t0 = 0;
}

bool RACProtocol::receive(bool updated_registers[]) {
  for (int i = 0; i < _NUM_REGISTERS; i++) {
    updated_registers[i] = false;
  }
  bool start_received = false;
  if (Serial.available() > 3) {
    byte bb[1];
    Serial.readBytes(bb, 1);
    start_received = bb[0] == 0xff;
    if (start_received) {
      bool end_received = false;

      while (!end_received) {
        Serial.readBytes(bb, 1);
        int current_register = bb[0];

        if (current_register == 0xfe) { // Make 0xfe constant
          end_received = true;
        } else {
          int register_size = _REGISTER_SIZE[current_register];
          byte input_buffer[register_size];
          Serial.readBytes(input_buffer, register_size);
          int start_index = _INDEX_ARRAY[current_register];
          for (int i = 0; i < register_size; i++) {
            _VALUE_BUFFER[start_index + i] = input_buffer[i];
          }
          updated_registers[current_register] = true;
        }
      }
    }
  }
  return start_received;
}

bool RACProtocol::receive_lite(bool updated_registers[]) {
  bool debug = false;
  byte bb[1];
  bool new_value = false;

  if (primed) {

    if (Serial.available() >= prime_amount) {
      byte input_buffer[prime_amount];
      Serial.readBytes(input_buffer, prime_amount);
      int start_index = _INDEX_ARRAY[primed_register];
      for (int i = 0; i < prime_amount; i++) {
        _VALUE_BUFFER[start_index + i] = input_buffer[i];
      }
      updated_registers[primed_register] = true;
      new_value = true;
      primed = false;
      receive_lite(updated_registers); // This line together with it's counterpart in the else statement might work to enable the light version to read multiple values at once.
      debug = false;
    }
  } else {
    if (Serial.available() > 0) {
      Serial.readBytes(bb, 1);
      primed_register = bb[0];
      prime_amount = _REGISTER_SIZE[primed_register];
      primed = true;
      new_value = receive_lite(updated_registers);
      debug = true;
    }
  }
  return new_value;
}

void RACProtocol::transmit() {
  Serial.write(0xff);
  for (int i = 0; i < _NUM_REGISTERS; i++) {
    write_out(i);
  }
  Serial.write(0xfe);
}

void RACProtocol::transmit(int registers[], int num) {
  Serial.write(0xff);
  for (int i = 0; i < num; i++) {
    int reg = registers[i];
    write_out(reg);
  }
  Serial.write(0xfe);
}

void RACProtocol::write_out(int reg) {
  int register_size = _REGISTER_SIZE[reg];
  int register_index = _INDEX_ARRAY[reg];
  Serial.write(reg);
  for (int j = 0; j < register_size; j++) {
    Serial.write(_VALUE_BUFFER[j + register_index]);
  }
}

bool RACProtocol::get_bool(int reg_num) {
  union {
    byte bytes;
    bool value;
  } UNION;
  int index = _INDEX_ARRAY[reg_num];
  UNION.bytes = _VALUE_BUFFER[index];
  return UNION.value;
}

byte RACProtocol::get_byte(int reg_num) {
  int index = _INDEX_ARRAY[reg_num];
  byte value = _VALUE_BUFFER[index];
  return value;
}

int RACProtocol::get_int(int reg_num) {
  union {
    byte bytes[2];
    int value;
  } UNION;
  int index = _INDEX_ARRAY[reg_num];
  for (int i = 0; i < 2; i++) {
    UNION.bytes[i] = _VALUE_BUFFER[index + i];
  }
  return UNION.value;
}

float RACProtocol::get_float(int reg_num) {
  union {
    byte bytes[4];
    float value;
  } UNION;
  int index = _INDEX_ARRAY[reg_num];
  for (int i = 0; i < 4; i++) {
    UNION.bytes[i] = _VALUE_BUFFER[index + i];
  }
  return UNION.value;
}

long RACProtocol::get_long(int reg_num) {
  union {
    byte bytes[4];
    long value;
  } UNION;
  int index = _INDEX_ARRAY[reg_num];
  for (int i = 0; i < 4; i++) {
    UNION.bytes[i] = _VALUE_BUFFER[index + i];
  }
  return UNION.value;
}

unsigned int RACProtocol::get_uint(int reg_num) {
  union {
    byte bytes[2];
    unsigned int value;
  } UNION;
  int index = _INDEX_ARRAY[reg_num];
  for (int i = 0; i < 2; i++) {
    UNION.bytes[i] = _VALUE_BUFFER[index + i];
  }
  return UNION.value;
}

unsigned long RACProtocol::get_ulong(int reg_num) {
  union {
    byte bytes[4];
    unsigned long value;
  } UNION;
  int index = _INDEX_ARRAY[reg_num];
  for (int i = 0; i < 4; i++) {
    UNION.bytes[i] = _VALUE_BUFFER[index + i];
  }
  return UNION.value;
}

void RACProtocol::write_bool(int reg_num, bool value) {
  union {
    byte bytes[1];
    bool value;
  } UNION;
  UNION.value = value;
  int index = _INDEX_ARRAY[reg_num];
  _VALUE_BUFFER[index];
}

void RACProtocol::write_byte(int reg_num, byte value) {
  int index = _INDEX_ARRAY[reg_num];
  _VALUE_BUFFER[index];
}

void RACProtocol::write_int(int reg_num, int value) {
  union {
    byte bytes[2];
    int value;
  } UNION;
  UNION.value = value;
  int index = _INDEX_ARRAY[reg_num];
  int register_size = _REGISTER_SIZE[reg_num];
  for (int i = 0; i < register_size; i++) {
    _VALUE_BUFFER[i + index] = UNION.bytes[i];
  }
}

void RACProtocol::write_float(int reg_num, float value) {
  union {
    byte bytes[4];
    float value;
  } UNION;
  UNION.value = value;
  int index = _INDEX_ARRAY[reg_num];
  int register_size = _REGISTER_SIZE[reg_num];
  for (int i = 0; i < register_size; i++) {
    _VALUE_BUFFER[i + index] = UNION.bytes[i];
  }
}

void RACProtocol::write_long(int reg_num, long value) {
  union {
    byte bytes[4];
    long value;
  } UNION;
  UNION.value = value;
  int index = _INDEX_ARRAY[reg_num];
  int register_size = _REGISTER_SIZE[reg_num];
  for (int i = 0; i < register_size; i++) {
    _VALUE_BUFFER[i + index] = UNION.bytes[i];
  }
}

void RACProtocol::write_uint(int reg_num, unsigned int value) {
  union {
    byte bytes[2];
    unsigned int value;
  } UNION;
  UNION.value = value;
  int index = _INDEX_ARRAY[reg_num];
  int register_size = _REGISTER_SIZE[reg_num];
  for (int i = 0; i < register_size; i++) {
    _VALUE_BUFFER[i + index] = UNION.bytes[i];
  }
}

void RACProtocol::write_ulong(int reg_num, unsigned long value) {
  union {
    byte bytes[4];
    unsigned long value;
  } UNION;
  UNION.value = value;
  int index = _INDEX_ARRAY[reg_num];
  int register_size = _REGISTER_SIZE[reg_num];
  for (int i = 0; i < register_size; i++) {
    _VALUE_BUFFER[i + index] = UNION.bytes[i];
  }
}
