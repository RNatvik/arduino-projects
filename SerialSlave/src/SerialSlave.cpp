
#include "Arduino.h"
#include <DataRegister.h>
#include "SerialSlave.h"


SerialSlave::SerialSlave() {
  init = false;
}

SerialSlave::SerialSlave(DataRegister data_register) {
  reg = data_register;
  init = true;
}

int SerialSlave::scan() {
  if (!init) {
    return -100;
  }
  int resp = 0;

  bool new_message = Serial.available() >= 3;
  if (new_message) {
    byte bytes[3];
    Serial.readBytes(bytes, 3);
    bool valid = bytes[0] == 0xff;
    byte function = bytes[1];
    byte num = bytes[2]; //word num = *(word*)&bytes[2];
    if (valid) {
      resp = 1;
      switch (function) {
        case SER_READ:
          handle_read(num);
          break;
        case SER_WRITE:
          handle_write(num);
          break;
        default:
          handle_invalid();
          resp = -2;
          break;
      }
    } else {
      handle_invalid();
      resp = -1;
    }
  }
  return resp;
}

void SerialSlave::set_register(DataRegister data_register) {
  reg = data_register;
  init = true;
}

void SerialSlave::handle_write(byte num) {
  byte primer[] = {SER_WRITE};
  Serial.write(primer, 1);
  for (word i = 0; i < num; i++) {
    byte reg_num[1];
    Serial.readBytes(reg_num, 1);
    Register reg_object = reg.get(reg_num[0]);
    byte bytes[reg_object.n];
    Serial.readBytes(bytes, reg_object.n);
    reg.put(bytes, reg_num[0]);
  }
}

void SerialSlave::handle_read(byte num) {
  byte primer[] = {SER_READ};
  Serial.write(primer, 1);
  for (word i = 0; i < num; i++) {
    byte reg_num[1];
    Serial.readBytes(reg_num, 1);
    Register reg_object = reg.get(reg_num[0]);
    Serial.write(reg_object.pointer, reg_object.n);
  }
}

void SerialSlave::handle_invalid() {
  int num_available = Serial.available();
  if (num_available > 0) {
    byte waste[num_available];
    Serial.readBytes(waste, num_available);
  }
  byte primer[] = {SER_ERROR};
  Serial.write(primer, 1);
}
