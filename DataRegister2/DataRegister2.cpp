#include "Arduino.h"
#include "DataRegister2.h"

DataRegister2::DataRegister2() {
}

DataRegister2::DataRegister2(int num_register, byte* value_buffer[], int size_array[]) {
  _VALUE_BUFFER = value_buffer;
  _SIZE_ARRAY = size_array;
  _NUM_REGISTERS = num_register;
}

void DataRegister2::link(int reg_num, byte* value, int reg_size) {
  _VALUE_BUFFER[reg_num] = value;
  _SIZE_ARRAY[reg_num] = reg_size;
}

Register DataRegister2::get(int reg_num) {
  Register reg = {
    _VALUE_BUFFER[reg_num],
    _SIZE_ARRAY[reg_num]
  };
  return reg;
}

void DataRegister2::put(byte* value, int reg_num) {
  int n = _SIZE_ARRAY[reg_num];
  for (int i = 0; i < n; i++) {
    _VALUE_BUFFER[reg_num][i] = value[i];
  }
}
