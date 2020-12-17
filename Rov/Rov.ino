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
// Global variables
//----------------------------------------------------
bool command_bits[16];
unsigned long scan_counter;

//----------------------------------------------------
// IMU
//----------------------------------------------------
const int   MPU_ADDR    = 0x68;
const int   ACC_BASE    = 0x3B;
const int   GYRO_BASE   = 0x43;
const int   TEMP_BASE   = 0x41;
const int ACC_OFFSET[]  = {1737, 239, -973};
const int GYRO_OFFSET[] = {-647, -90, -172};
const int MAG_OFFSET[]  = {0, 0, 0};

//----------------------------------------------------
// Serial Slave protocol
//----------------------------------------------------
SerialSlave serial_slave;

//----------------------------------------------------
// DataRegister
//----------------------------------------------------
const int num_register = 23;
const int num_buffer = 2 + 2 + 2 + 4 + 4 + 4 + 1 + 1 + 1 + 4 + 4 + 4 + 2 + 2 + 2 + 2 + 2 + 2 + 2 + 2 + 2 + 4 + 4;
// REGISTER NUMBERS             0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22
int size_array[num_register] = {2, 2, 2, 4, 4, 4, 1, 1, 1, 4, 4, 4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 4, 4};
int index_array[num_register];
byte buffer_array[num_buffer];
DataRegister data_register;

//----------------------------------------------------
// Registers
//----------------------------------------------------
word*  cw;
word*  sw;
int*   loop_time;
float* battery_level;
float* temp_left;
float* temp_right;
byte*  speed_left;
byte*  speed_right;
byte*  speed_pitch;
float* roll;
float* pitch;
float* yaw;
int*   rax;
int*   ray;
int*   raz;
int*   rgx;
int*   rgy;
int*   rgz;
int*   rmx;
int*   rmy;
int*   rmz;
float* comp_alpha;
float* temp_main;

//****************************************************
// SETUP
//****************************************************
void setup() {
  // ----
  // Serial Register
  // ----
  Serial.begin(115200);
  data_register = DataRegister(num_buffer, num_register, buffer_array, size_array, index_array);
  cw            = (word*)  data_register.link(0);  *cw            = 0x0000;
  sw            = (word*)  data_register.link(1);  *sw            = 0x0000;
  loop_time     = (int*)   data_register.link(2);  *loop_time     = 10;
  battery_level = (float*) data_register.link(3);  *battery_level = 0;
  temp_left     = (float*) data_register.link(4);  *temp_left     = 0;
  temp_right    = (float*) data_register.link(5);  *temp_right    = 0;
  speed_left    = (byte*)  data_register.link(6);  *speed_left    = 0;
  speed_right   = (byte*)  data_register.link(7);  *speed_right   = 0;
  speed_pitch   = (byte*)  data_register.link(8);  *speed_pitch   = 0;
  roll          = (float*) data_register.link(9);  *roll          = 0;
  pitch         = (float*) data_register.link(10); *pitch         = 0;
  yaw           = (float*) data_register.link(11); *yaw           = 0;
  rax           = (int*)   data_register.link(12); *rax           = 0;
  ray           = (int*)   data_register.link(13); *ray           = 0;
  raz           = (int*)   data_register.link(14); *raz           = 0;
  rgx           = (int*)   data_register.link(15); *rgx           = 0;
  rgy           = (int*)   data_register.link(16); *rgy           = 0;
  rgz           = (int*)   data_register.link(17); *rgz           = 0;
  rmx           = (int*)   data_register.link(18); *rmx           = 0;
  rmy           = (int*)   data_register.link(19); *rmy           = 0;
  rmz           = (int*)   data_register.link(20); *rmz           = 0;
  comp_alpha    = (float*) data_register.link(21); *comp_alpha    = 0.95;
  temp_main     = (float*) data_register.link(22); *temp_main     = 0;
  serial_slave.set_register(data_register);

  // ----
  // IMU
  // ----
  Wire.begin();
  imu_wakeup();

  // ----
  // Serial Register
  // ----
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

  //Unpack command
  unpack_cw();

  //io();
  imu();

  pack_sw();

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
  // Input
  *battery_level  = fMap(analogRead(VOLTAGE_MONITOR_PIN)   , 0, 1023, 0, 20);
  *temp_left      = fMap(analogRead(TEMP_LEFT_BATTERY_PIN) , 0, 1023, 0, 80);
  *temp_right     = fMap(analogRead(TEMP_RIGHT_BATTERY_PIN), 0, 1023, 0, 80);
  water_guard_ok1 = digitalRead(WATER_GUARD_PIN1);
  water_guard_ok2 = digitalRead(WATER_GUARD_PIN2);
  water_guard_ok3 = digitalRead(WATER_GUARD_PIN3);

  // Output
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
  word status_word = *sw;
  
  bool loop_error = scan_counter < 1;
  bool battery_warning = (*battery_level < 12.8) ||
                         (*temp_left > 30)       ||
                         (*temp_right > 30);

  bool battery_error   = (*battery_level < 12) ||
                         (*temp_left > 45) ||
                         (*temp_right > 45);
  status_word = loop_error << 3 | battery_error << 2 | battery_warning << 1;
  *sw = status_word;
}
//----------------------------------------------------
// IMU
//----------------------------------------------------
void imu() {
  imu_read();
  calculate_orientation();
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
//----------------------------
//  IMU FUNCTIONS
//----------------------------
void imu_wakeup() {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission();
}

void imu_read() {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(ACC_BASE);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 3 * 2);
  *rax = Wire.read() << 8 | Wire.read();
  *ray = Wire.read() << 8 | Wire.read();
  *raz = Wire.read() << 8 | Wire.read();

  Wire.beginTransmission(MPU_ADDR);
  Wire.write(GYRO_BASE);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 3 * 2);
  *rgx = Wire.read() << 8 | Wire.read();
  *rgy = Wire.read() << 8 | Wire.read();
  *rgz = Wire.read() << 8 | Wire.read();

  Wire.beginTransmission(MPU_ADDR);
  Wire.write(TEMP_BASE);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 2);
  int rtemp = Wire.read() << 8 | Wire.read();
  *temp_main = (float) rtemp / 340 + 36.53;
}

void calculate_orientation() {
  int min_scale = -32760;
  int max_scale = 32760;
  float acc_x =  lin_map(*rax, min_scale, max_scale, -2,    2,    ACC_OFFSET[0]);
  float acc_y =  lin_map(*ray, min_scale, max_scale, -2,    2,    ACC_OFFSET[1]);
  float acc_z =  lin_map(*raz, min_scale, max_scale, -2,    2,    ACC_OFFSET[2]);
  float gyro_x = lin_map(*rgx, min_scale, max_scale, -250,  250,  GYRO_OFFSET[0]);
  float gyro_y = lin_map(*rgy, min_scale, max_scale, -250,  250,  GYRO_OFFSET[1]);
  float gyro_z = lin_map(*rgz, min_scale, max_scale, -250,  250,  GYRO_OFFSET[2]);
  float mag_x =  lin_map(*rmx, min_scale, max_scale, -4912, 4912, MAG_OFFSET[0]);
  float mag_y =  lin_map(*rmy, min_scale, max_scale, -4912, 4912, MAG_OFFSET[1]);
  float mag_z =  lin_map(*rmz, min_scale, max_scale, -4912, 4912, MAG_OFFSET[2]);

  float acc_roll = atan2(acc_y, acc_z) * 180.0 / PI; // Converted to degrees
  float acc_pitch = atan2(-acc_x, sqrt(pow(acc_y, 2) + pow(acc_z, 2))) * 180.0 / PI;
  float mag_yaw = atan2(mag_y, mag_x) * 180.0 / PI;

  float dt = (float) *loop_time / 1000;
  *roll  = *comp_alpha * (*roll  + gyro_x * dt) + (1 - *comp_alpha) * acc_roll;
  *pitch = *comp_alpha * (*pitch + gyro_y * dt) + (1 - *comp_alpha) * acc_pitch;
  *yaw   = *comp_alpha * (*yaw   + gyro_z * dt) + (1 - *comp_alpha) * mag_yaw;
}

float lin_map(float x, float x0, float x1, float y0, float y1) {
  return (x - x0) * (y1 - y0) / (x1 - x0) + y0;
}

float lin_map(float x, float x0, float x1, float y0, float y1, float offs) {
  return lin_map(x - offs, x0, x1, y0, y1);
}
