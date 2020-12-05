#ifndef SerialSlave_h
#define SerialSlave_h

#define SER_READ 0xaa
#define SER_WRITE 0xbb
#define SER_ERROR 0xee

#include "Arduino.h"
#include <DataRegister.h>

class SerialSlave {
  public:
    SerialSlave();
    SerialSlave(DataRegister reg);

    void set_register(DataRegister data_register);
    int scan();
    
  private:
    DataRegister reg;
    bool init;
    void handle_write(byte num);
    void handle_read(byte num);
    void handle_invalid();
};



#endif
