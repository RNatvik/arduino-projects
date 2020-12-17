#ifndef DataRegister2_h
#define DataRegister2_h

#include "Arduino.h"

struct Register {
  byte * pointer;
  int n;
};

class DataRegister2 {
  public:
    DataRegister2();
    DataRegister2(int num_register, byte* value_buffer[], int size_array[]);

    void link(int reg_num, byte* value, int reg_size);
    Register get(int reg_num);
    void put(byte* value, int reg_num);

    
  private:
    byte **_VALUE_BUFFER; // A buffer where incoming messages are stored in corresponding registers
    int *_SIZE_ARRAY; // An array containing the size of the corresponding registers' values (register_size[2] = 4 means that the third register has a size of 4 i.e float, long etc)
    int _NUM_REGISTERS;
};



#endif
