#ifndef SerialProtocol_h
#define SerialProtocol_h

#include "Arduino.h"

/*
 * How will this protocol function?
 * Transmit only on request?
 * Stream out?
 * Both?
 * Client toggle stream or request functionality?
 * Read / write permission?
 */
class SerialProtocol {
  public:
    SerialProtocol();
    SerialProtocol(unsigned long baudrate, int num_buffer, int num_register, byte value_buffer[], int size_array[], int index_array[]);

    
  private:
    byte *_VALUE_BUFFER; // A buffer where incoming messages are stored in corresponding registers
    int *_SIZE_ARRAY; // An array containing the size of the corresponding registers' values (register_size[2] = 4 means that the third register has a size of 4 i.e float, long etc)
    int *_INDEX_ARRAY; // An array containing the cumulative sum of the sizes of data in the register_size array. This can be used to determine where to locate a given register in the value_buffer
    int _NUM_REGISTERS;
    int _NUM_BUFFER;
};

#endif
