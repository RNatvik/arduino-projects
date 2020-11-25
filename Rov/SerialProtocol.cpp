#include "Arduino.h"
#include "SerialProtocol.h"

SerialProtocol::SerialProtocol() {
}

SerialProtocol::SerialProtocol(unsigned long baudrate, int num_buffer, int num_register, byte value_buffer[], int size_array[], int index_array[]) {
  _VALUE_BUFFER = value_buffer;
  _SIZE_ARRAY = size_array;
  _INDEX_ARRAY = index_array;
  _NUM_REGISTERS = num_register;
  _NUM_BUFFER = num_buffer;
  int cumsum = 0;
  for (int i = 0; i < num_register; i++) {
    _INDEX_ARRAY[i] = cumsum;
    cumsum += _SIZE_ARRAY[i];
  }
  for (int i = 0; i < num_buffer; i++) {
    _VALUE_BUFFER[i] = 0;
  }
}
