#include <DataRegister.h>
#include <SerProt.h>

//----------------------------------
// Data Register
//----------------------------------
const int buffer_add = 50;

const int num_register = 4 + buffer_add;
const int num_buffer = 2 + 4 + 4 + 4 + buffer_add*2;
int size_array[num_register];
int index_array[num_register];
byte buffer_array[num_buffer];
DataRegister reg; 


//----------------------------------
// Protocol
//----------------------------------
Protocol prot;

//----------------------------------
// Test
//----------------------------------
unsigned long t0 = 0;
unsigned long c = 0;

unsigned int * r0;
float * r1;
unsigned long * r2;
unsigned long* r3;

void setup() {
  size_array[0] = 2; size_array[1] = 4; size_array[2] = 4; size_array[3] = 4;
  for (int i = 4; i < num_register; i++) {
    size_array[i] = 2;
  }
  
  Serial.begin(500000);
  reg = DataRegister(num_buffer, num_register, buffer_array, size_array, index_array);
  prot.set_register(reg);

  r0 = (unsigned int*) reg.link(0); *r0 = 1337;
  r1 = (float*) reg.link(1); *r1 = 12.23;
  r2 = (unsigned long*) reg.link(2); *r2 = 0;
  r3 = (unsigned long*) reg.link(3); *r3 = 0;
//  for (word i = 0; i < buffer_add; i++) {
//    reg.put((byte*) i, i+3);
//  }
  t0 = micros();
}

void loop() {
  unsigned long t = micros();
  *r2 = t - t0;
  t0 = t;
  unsigned long t2 = micros();
  int ret = prot.scan();
  unsigned long t3 = micros();
  if (ret == 1) {
    c += 1;
    if (c % 1 == 0) {
      *r3 = t3 - t2;
    }
  }
  //Serial.print(*r0); Serial.print(','); Serial.print(*r1); Serial.print(','); Serial.println(*r2);
}
