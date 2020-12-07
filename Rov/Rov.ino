#include <DataRegister.h>
#include <SerialSlave.h>

//----------------------------------------------------
// DataRegister
//----------------------------------------------------
const int num_register = 20;
const int num_buffer = 50;
byte buffer_array[num_buffer];
int size_array[num_register];
int index_array[num_register];
DataRegister data_register = DataRegister(num_buffer, num_register, buffer_array, size_array, index_array);

//----------------------------------------------------
// Registers
//----------------------------------------------------
word* cw;
word* sw;
int* loop_time;

//----------------------------------------------------
// Serial Slave protocol
//----------------------------------------------------
SerialSlave serial_slave = SerialSlave(data_register);

//----------------------------------------------------
// Variables
//----------------------------------------------------
bool command_bits[16];
unsigned long scan_counter;


//****************************************************
// SETUP
//****************************************************
void setup() {
  Serial.begin(115200);

  cw        = (word*) data_register.link(0, 2); *cw        = 0x0000;
  sw        = (word*) data_register.link(1, 2); *sw        = 0x0000;
  loop_time = (int*)  data_register.link(2, 2); *loop_time = 10;
  data_register.lock();

  scan_counter = 1;
}

//****************************************************
// LOOP
//****************************************************
void loop() {
  unsigned long loop_start = millis();

  //Set status
  bool loop_time_error = scan_counter == 0;
  set_status(loop_time_error, 0);
  
  //Unpack command
  unpack_cw();

  scan_counter = 0;
  serial_slave.scan();
  while (millis() < loop_start + *loop_time) {
    scan_counter++;
    serial_slave.scan();
  }
}

void unpack_cw() {
  union {
    word w;
    bool bits[16];
  } uni;
  uni.w = *cw;
  for (int i = 0; i < 16; i++) {
    command_bits[i] = uni.bits[i];
  }
}

void set_status(bool value, int bit_number) {
  *sw = *sw | value << bit_number;
}
