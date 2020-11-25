#ifndef NTDProtocol_h
#define NTDProtocol_h

#include "Arduino.h"

class NTDProtocol
{
  public:
    NTDProtocol();
    void write_header(int n, byte types[]);
    void write_data(bool input_value);
    void write_data(byte input_value);
    void write_data(int input_value);
    void write_data(long input_value);
    void write_data(float input_value);
    void write_data(unsigned int input_value);
    void write_data(unsigned long input_value);
    void write_end();

    bool receive_data(byte data_buffer[]);

    byte bool_code();
    byte byte_code();
    byte int_code();
    byte long_code();
    byte float_code();
    byte uint_code();
    byte ulong_code();


  private:
    static const byte _BOOL = 0x01;
    static const byte _BYTE = 0x02;
    static const byte _INT = 0x03;
    static const byte _LONG = 0x04;
    static const byte _FLOAT = 0x05;
    static const byte _UINT = 0x06;
    static const byte _ULONG = 0x07;

    bool fill_data(int n, byte type_buffer[], byte data_buffer[]);
};

#endif
