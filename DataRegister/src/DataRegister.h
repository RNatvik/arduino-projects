#ifndef DataRegister_h
#define DataRegister_h

#include "Arduino.h"

struct Register {
  byte * pointer;
  int n;
};

class DataRegister {
  public:
    DataRegister();
    DataRegister(int num_buffer, int num_register, byte value_buffer[], int size_array[], int index_array[]);

    byte * link(int reg_num, int size);
    Register get(int reg_num);
    void put(byte* value, int reg_num);
    void lock();

    
  private:
    byte *_VALUE_BUFFER; // A buffer where incoming messages are stored in corresponding registers
    int *_SIZE_ARRAY; // An array containing the size of the corresponding registers' values (register_size[2] = 4 means that the third register has a size of 4 i.e float, long etc)
    int *_INDEX_ARRAY; // An array containing the cumulative sum of the sizes of data in the register_size array. This can be used to determine where to locate a given register in the value_buffer
    int _NUM_REGISTERS;
    int _NUM_BUFFER;
};



#endif
