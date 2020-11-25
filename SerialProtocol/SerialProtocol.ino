#include "NTDProtocol.h"

NTDProtocol protocol = NTDProtocol();

// Test Indicator
int pin = 13;
int target_int = 1023;
float target_float = 12.52;
long target_long = 120000;
bool state = false;

// Test Data
bool bool_value = true;
byte byte_value = 0x56;
int int_value = 1356;
long long_value = 100000;
float float_value = 123.5;
unsigned int uint_value = 40000;
unsigned long ulong_value = 3000000;

int num = 7;
byte type_array[] = {protocol.bool_code(), protocol.byte_code(), protocol.int_code(), protocol.long_code(), protocol.float_code(), protocol.uint_code(), protocol.ulong_code()};

void setup() {
  pinMode(pin, OUTPUT);
  digitalWrite(pin, state);
  Serial.begin(250000);
}

void loop() {
  unsigned long a0 = micros();
  transmit_function();
  unsigned long a = micros();
  unsigned long dta = a - a0;

  unsigned long b0 = micros();
  byte message_buffer[20];
  bool new_msg = protocol.receive_data(message_buffer);
  if (new_msg) {
    receive_function(message_buffer);
  }
  unsigned long b = micros();
  unsigned long dtb = b - b0;
  Serial.println();
  Serial.print(dta);
  Serial.print("   ");
  Serial.println(dtb);
}

void transmit_function() {
  protocol.write_header(num, type_array);
  protocol.write_data(bool_value);
  protocol.write_data(byte_value);
  protocol.write_data(int_value);
  protocol.write_data(long_value);
  protocol.write_data(float_value);
  protocol.write_data(uint_value);
  protocol.write_data(ulong_value);
  protocol.write_end();
}

void receive_function(byte message_buffer[]) {
  union {
    byte bytes[2];
    int value;
  } INT_UNION;
  union {
    byte bytes[4];
    float value;
  } FLOAT_UNION;
  union {
    byte bytes[4];
    unsigned long value;
  } LONG_UNION;

  INT_UNION.bytes[0] = message_buffer[4];
  INT_UNION.bytes[1] = message_buffer[5];
  FLOAT_UNION.bytes[0] = message_buffer[6];
  FLOAT_UNION.bytes[1] = message_buffer[7];
  FLOAT_UNION.bytes[2] = message_buffer[8];
  FLOAT_UNION.bytes[3] = message_buffer[9];
  LONG_UNION.bytes[0] = message_buffer[10];
  LONG_UNION.bytes[1] = message_buffer[11];
  LONG_UNION.bytes[2] = message_buffer[12];
  LONG_UNION.bytes[3] = message_buffer[13];

  int int_value = INT_UNION.value;
  float float_value = INT_UNION.value;
  long long_value = INT_UNION.value;

  if (int_value == target_int && float_value == target_float && long_value == target_long) {
    state = not state;
    digitalWrite(pin, state);
  }
}
