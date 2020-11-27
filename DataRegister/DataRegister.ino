#include "DataRegister.h"

float * my_float;
int * my_int;
word * my_word;

const int num_registers = 3;
const int buffer_size = 4 + 2 + 2; // float + int + word
int size_array[] = {4, 2, 2};
int index_array[num_registers];
byte value_buffer[buffer_size];

void setup() {
  Serial.begin(115200);
  
  DataRegister data_register = DataRegister(buffer_size, num_registers, value_buffer, size_array, index_array);
  
  my_float = (float*) data_register.link(0); *my_float = -24.23;
  my_int   = (int*)   data_register.link(1); *my_int  = 1337;
  my_word  = (word*)  data_register.link(2); *my_word  = 0x8008;

  Serial.println(1); Serial.print(*my_float); Serial.print(", "); Serial.print(*my_int); Serial.print(", "); Serial.println(*my_word);
  float new_float = 12.34;
  int new_int = 8008;
  word new_word = 1337;
  data_register.put((byte*)&new_float, 0);
  data_register.put((byte*)&new_int, 1);
  data_register.put((byte*)&new_word, 2);
  Serial.println(2); Serial.print(*my_float); Serial.print(", "); Serial.print(*my_int); Serial.print(", "); Serial.println(*my_word);

  byte word_bytes[] = {0x08, 0x80};
  data_register.put(word_bytes, 2);
  Serial.println(3); Serial.print(*my_float); Serial.print(", "); Serial.print(*my_int); Serial.print(", "); Serial.println(*my_word);

  

  byte twoWords[] = {0x80, 0x08, 0x69, 0x69};
  data_register.put(twoWords, 2);
  Serial.println(4); Serial.print(*my_float); Serial.print(", "); Serial.print(*my_int); Serial.print(", "); Serial.println(*my_word);
  
  data_register.put(&twoWords[2], 2);
  Serial.println(5); Serial.print(*my_float); Serial.print(", "); Serial.print(*my_int); Serial.print(", "); Serial.println(*my_word);

  word_bytes[0] = 0x69;
  word_bytes[1] = 0x69;
  data_register.put(word_bytes, 2);
  Serial.println(6); Serial.print(*my_float); Serial.print(", "); Serial.print(*my_int); Serial.print(", "); Serial.println(*my_word);
  
  
  
}

void loop() {
  // put your main code here, to run repeatedly:

}
