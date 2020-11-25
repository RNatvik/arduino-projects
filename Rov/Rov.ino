#include "SerialProtocol.h"
//----------------------------
//  SERIAL
//----------------------------
SerialProtocol protocol;

// Protocol variables
const int num_registers = 7;
const int buffer_size = 28; // 7x float
int register_size[] = {4, 4, 4, 4, 4, 4, 4};
int index_array[num_registers];
byte value_buffer[buffer_size];

//----------------------------
//  REGISTERS
//  These are variables linked to the serial communication registers
//----------------------------
float * kp;
float * ki;
float * kd;
float * target;
float * roll;
float * pitch;
float * yaw;

void setup() {
  protocol = SerialProtocol(250000, buffer_size, value_buffer, num_registers, register_size, index_array);
  // Assign serial registers and set initial values. "index_array" has been calculated by construction of RACProtocol
  kp     = (float*)        &value_buffer[index_array[0]]; *kp     = 0;
  ki     = (float*)        &value_buffer[index_array[1]]; *ki     = 0;
  kd     = (float*)        &value_buffer[index_array[2]]; *kd     = 0;
  target = (float*)        &value_buffer[index_array[3]]; *target = 0;
  roll   = (float*)        &value_buffer[index_array[4]]; *roll   = 0;
  pitch  = (float*)        &value_buffer[index_array[5]]; *pitch  = 0;
  yaw    = (float*)        &value_buffer[index_array[6]]; *yaw    = 0;
}

void loop() {
}
