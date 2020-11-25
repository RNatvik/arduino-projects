#ifndef RACProtocol_h
#define RACProtocol_h

#include "Arduino.h"

class RACProtocol
/**
   Message: {
              Register
              Value
             }
*/
{
  public:
    RACProtocol();
    RACProtocol(int num_buffer, byte value_buffer[], int num_register, int register_size[], int index_array[]);

    bool receive(bool updated_registers[]); // Reads serial input. Returns true if new message was received false if no input
    bool receive_lite(bool updated_registers[]); // Faster than receive(), but needs a minimum of 2 cycles to extract a value (prime cycle + write cycle for each register) Only reads input after Serial.available() returns an amount equal or greater than the register size
    void transmit();

    bool get_bool(int reg_num);
    byte get_byte(int reg_num);
    int get_int(int reg_num);
    long get_long(int reg_num);
    float get_float(int reg_num);
    unsigned int get_uint(int reg_num);
    unsigned long get_ulong(int reg_num);

  private:
    byte *_VALUE_BUFFER; // A buffer where incoming messages are stored in corresponding registers
    int *_REGISTER_SIZE; // An array containing the size of the corresponding registers' values (register_size[2] = 4 means that the third register has a size of 4 i.e float, long etc)
    int *_INDEX_ARRAY; // An array containing the cumulative sum of the sizes of data in the register_size array. This can be used to determine where to locate a given register in the value_buffer
    int _NUM_REGISTERS;
    int _NUM_BUFFER;

    bool primed;
    int prime_amount;
    int primed_register;

    unsigned long t0;


    /**
       register_size[] = {2, 4, 2, 4}
       index_array[] = {0, 2, 6, 8} // First index is always 0, last size is ignored

       Message comes in:  | 0x02 | 0xFF | 0xA9 |
       Code reads the first byte (0x02)
       This message is destined for register 2 (0x02)
       Code looks up register_size[2] and finds the value 2 which is the number of bytes to read next
       Code reads the next 2 bytes
       The code looks up index_array[2] which has the value 6.
       The code writes 0xFF to value_buffer[6] and 0xA9 to value_buffer[7]

    */
};

#endif
