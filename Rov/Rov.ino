#include <DataRegister.h>
#include <SerialSlave.h>
#include <Wire.h>

//----------------------------------------------------
// IO Pins
//----------------------------------------------------
// Analog Input
const int TEMP_LEFT_BATTERY_PIN  = A1;
const int TEMP_RIGHT_BATTERY_PIN = A2;
const int VOLTAGE_MONITOR_PIN    = A3;

// Digital Input
const int WATER_GUARD_PIN1 = 3;
const int WATER_GUARD_PIN2 = 4;
const int WATER_GUARD_PIN3 = 5;

// Analog Output
const int FW_LEFT_MOTOR_PIN  = 9;
const int FW_RIGHT_MOTOR_PIN = 10;
const int PITCH_MOTOR_PIN    = 11;

// Digital Output
const int LIGHT_PIN = 6;

//----------------------------------------------------
// IO Variables
//----------------------------------------------------
bool water_guard_ok1;
bool water_guard_ok2;
bool water_guard_ok3;

bool light_active;

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
float* battery_level;
float* temp_left;
float* temp_right;
byte* speed_left;
byte* speed_right;
byte* speed_pitch;


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

  cw            = (word*)  data_register.link(0, 2); *cw            = 0x0000;
  sw            = (word*)  data_register.link(1, 2); *sw            = 0x0000;
  loop_time     = (int*)   data_register.link(2, 2); *loop_time     = 10;
  battery_level = (float*) data_register.link(3, 4); *battery_level = 0;
  temp_left     = (float*) data_register.link(4, 4); *temp_left     = 0;
  temp_right    = (float*) data_register.link(5, 4); *temp_right    = 0;
  speed_left    = (byte*)  data_register.link(6, 1); *speed_left    = 0;
  speed_right   = (byte*)  data_register.link(7, 1); *speed_right   = 0;
  speed_pitch   = (byte*)  data_register.link(8, 1); *speed_pitch   = 0;
  data_register.lock();

  pinMode(TEMP_LEFT_BATTERY_PIN, INPUT);
  pinMode(TEMP_RIGHT_BATTERY_PIN, INPUT);
  pinMode(VOLTAGE_MONITOR_PIN, INPUT);
  pinMode(WATER_GUARD_PIN1, INPUT);
  pinMode(WATER_GUARD_PIN2, INPUT);
  pinMode(WATER_GUARD_PIN3, INPUT);

  pinMode(FW_LEFT_MOTOR_PIN, OUTPUT);
  pinMode(FW_RIGHT_MOTOR_PIN, OUTPUT);
  pinMode(PITCH_MOTOR_PIN, OUTPUT);
  pinMode(LIGHT_PIN, OUTPUT);

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

  io();

  scan_counter = 0;
  serial_slave.scan();
  while (millis() < loop_start + *loop_time) {
    scan_counter++;
    serial_slave.scan();
  }
}

//****************************************************
// Programs
//****************************************************
//----------------------------------------------------
// IO
//----------------------------------------------------
void io() {
  // Analog
  *battery_level  = fMap(analogRead(VOLTAGE_MONITOR_PIN)   , 0, 1023, 0, 20);
  *temp_left      = fMap(analogRead(TEMP_LEFT_BATTERY_PIN) , 0, 1023, 0, 80);
  *temp_right     = fMap(analogRead(TEMP_RIGHT_BATTERY_PIN), 0, 1023, 0, 80);
  water_guard_ok1 = digitalRead(WATER_GUARD_PIN1);
  water_guard_ok2 = digitalRead(WATER_GUARD_PIN2);
  water_guard_ok3 = digitalRead(WATER_GUARD_PIN3);

  // Digital
  analogWrite(FW_LEFT_MOTOR_PIN,  *speed_left);
  analogWrite(FW_RIGHT_MOTOR_PIN, *speed_right);
  analogWrite(PITCH_MOTOR_PIN,    *speed_pitch);
  digitalWrite(LIGHT_PIN, light_active);

}
//----------------------------------------------------
// Unpack control word
//----------------------------------------------------
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
//----------------------------------------------------
// Pack status word
//----------------------------------------------------
void pack_sw() {
  union {
    word w;
    bool bits[16];
  } uni;

  bool battery_warning = (*battery_level < 12.8) ||
                         (*temp_left > 30)       ||
                         (*temp_right > 30);

  bool battery_error   = (*battery_level < 12) ||
                         (*temp_left > 45) ||
                         (*temp_right > 45);

  uni.bits[0] = false; // fellesfeil
  uni.bits[1] = battery_warning;
  uni.bits[2] = battery_error;


  *sw = uni.w;
}

//****************************************************
// Functions
//****************************************************
void set_status(bool value, int bit_number) {
  *sw = *sw | value << bit_number;
}

float fMap(float x, float x0, float x1, float y0, float y1) {
  return (x - x0) / (x1 - x0) * (y1 - y0) + y0;
}
