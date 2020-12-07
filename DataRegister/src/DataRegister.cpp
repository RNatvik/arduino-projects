#include "Arduino.h"
#include "DataRegister.h"

DataRegister::DataRegister() {
}

DataRegister::DataRegister(int num_buffer, int num_register, byte value_buffer[], int size_array[], int index_array[]) {
  _VALUE_BUFFER = value_buffer;
  _SIZE_ARRAY = size_array;
  _INDEX_ARRAY = index_array;
  _NUM_REGISTERS = num_register;
  _NUM_BUFFER = num_buffer;

  for (int i = 0; i < _NUM_REGISTERS; i++) {
    _SIZE_ARRAY[i] = 0;
    _INDEX_ARRAY[i] = 0;
  }
  for (int i = 0; i < _NUM_BUFFER; i++) {
    _VALUE_BUFFER[i] = 0;
  }
  
}

byte * DataRegister::link(int reg_num, int size) {
  _SIZE_ARRAY[reg_num] = size;
  return & _VALUE_BUFFER[_INDEX_ARRAY[reg_num]];
}

Register DataRegister::get(int reg_num) {
  Register reg = {
    & _VALUE_BUFFER[_INDEX_ARRAY[reg_num]],
    _SIZE_ARRAY[reg_num]
  };
  return reg;
}

void DataRegister::put(byte* value, int reg_num) {
  int n = _SIZE_ARRAY[reg_num];
  for (int i = 0; i < n; i++) {
    _VALUE_BUFFER[_INDEX_ARRAY[reg_num] + i] = value[i];
  }
}

void DataRegister::lock() {
  int cumsum = 0;
  for (int i = 0; i < _NUM_REGISTERS; i++) {
    _INDEX_ARRAY[i] = cumsum;
    cumsum += _SIZE_ARRAY[i];
  }
}
