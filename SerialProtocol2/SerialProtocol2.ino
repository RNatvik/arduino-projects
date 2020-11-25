#include "RACProtocol.h"

const int num_registers = 4;
const int buffer_size = 10;
int register_size[] = {2, 2, 2, 4};
int index_array[num_registers];
byte value_buffer[buffer_size];

RACProtocol protocol;

// Test variables
int my_var1 = 0;
int my_var2 = 0;
int my_var3 = 0;
float my_var4 = 0.0;

// Specify which registers (regX) correspond to which program variables
int *reg0 = &my_var1; // Set register 0 to correspond to "my_var1"
int *reg1 = &my_var2;
int *reg2 = &my_var3;
float *reg3 = &my_var4;

void setup() {
  Serial.begin(9600);
  Serial.println("Waiting...");
  protocol = RACProtocol(buffer_size, value_buffer, num_registers, register_size, index_array);
  Serial.println("Starting");
}

void loop() {
  bool updated_registers[num_registers];
  memset(updated_registers, 0, num_registers);
  bool new_values = protocol.receive_lite(updated_registers);
  if (new_values) {
    unsigned long t0 = micros();
    update_values(updated_registers);
    unsigned long t1 = micros();
    protocol.transmit();
    unsigned long t2 = micros();
    Serial.println();
    unsigned long t3 = micros();
    Serial.print(my_var1); Serial.print(", "); Serial.print(my_var2); Serial.print(", "); Serial.print(my_var3); Serial.print(", "); Serial.println(my_var4);
    unsigned long t4 = micros();

    unsigned long dt1 = t1 - t0;
    unsigned long dt2 = t2 - t1;
    unsigned long dt3 = t3 - t2;
    unsigned long dt4 = t4 - t3;

    Serial.println(dt1);
    Serial.println(dt2);
    Serial.println(dt3);
    Serial.println(dt4);
  }
}

void update_values(bool updated_registers[]) {
  for (int i = 0; i < num_registers; i++) {
    bool updated = updated_registers[i];
    if (updated) {
      switch (i) {
        case 0:
          *reg0 = protocol.get_int(0);
          break;
        case 1:
          *reg1 = protocol.get_int(1);
          break;
        case 2:
          *reg2 = protocol.get_int(2);
          break;
        case 3:
          *reg3 = protocol.get_float(3);
          break;
      }
      updated_registers[i] = false;
    }
  }
}
