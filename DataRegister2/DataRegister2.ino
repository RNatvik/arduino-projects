#include "DataRegister2.h"

int test_i;
float test_f;
word test_w;

const int num_registers = 3;
byte* pointers[num_registers];
int sizes[num_registers];
DataRegister2 data_register = DataRegister2(num_registers, pointers, sizes);

void setup() {
  // put your setup code here, to run once:
  test_i = 123;
  test_f = 213.56;
  test_w = 8008;
  
  Serial.begin(115200);
  data_register.link(0, (byte*)&test_i, 2);
  data_register.link(1, (byte*)&test_f, 4);
  data_register.link(2, (byte*)&test_w, 2);

  Register ri = data_register.get(0);
  Register rf = data_register.get(1);
  Register rw = data_register.get(2);

  for (int i = 0; i < ri.n; i++) {
    Serial.print(ri.pointer[i], 16); Serial.print(',');
  }
  Serial.println();
  for (int i = 0; i < rf.n; i++) {
    Serial.print(rf.pointer[i], 16); Serial.print(',');
  }
  Serial.println();
  for (int i = 0; i < rw.n; i++) {
    Serial.print(rw.pointer[i], 16); Serial.print(',');
  }
  Serial.println();
  Serial.println();

  int leet = 1337;
  data_register.put( (byte*) &leet, 0);
  ri = data_register.get(0);
  rf = data_register.get(1);
  rw = data_register.get(2);
  for (int i = 0; i < ri.n; i++) {
    Serial.print(ri.pointer[i], 16); Serial.print(',');
  }
  Serial.println();
  for (int i = 0; i < rf.n; i++) {
    Serial.print(rf.pointer[i], 16); Serial.print(',');
  }
  Serial.println();
  for (int i = 0; i < rw.n; i++) {
    Serial.print(rw.pointer[i], 16); Serial.print(',');
  }
  Serial.println();
}

void loop() {
  // put your main code here, to run repeatedly:

}
