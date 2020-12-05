#include <DataRegister.h>
#include <SerialSlave.h>
#include <Wire.h>

//---------------------------------
// IMU
//---------------------------------
const int MPU_ADDR = 0x68;
float comp_alpha = 0.98; // Complementary filter alpha
int acc_offset[] = {0, 0, 0};
int gyro_offset[] = {0, 0, 0};
int mag_offset[] = {0, 0, 0};
int acc_scale = 16384;
int gyro_scale = 131;

//---------------------------------
// Serial
//---------------------------------
DataRegister reg;
SerialSlave ser_slave;

const int num_regs = 13;
const int num_buffer = 4 + 4 + 4 + 2 + (2 * 9);
int size_array[] = {4, 4, 4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2};
int index_array[num_regs];
byte buffer_array[num_buffer];

//---------------------------------
// Registers
//---------------------------------
float* roll;
float* pitch;
float* yaw;
unsigned int* loop_time;
int* rax;
int* ray;
int* raz;
int* rgx;
int* rgy;
int* rgz;
int* rmx;
int* rmy;
int* rmz;

void setup() {
  Serial.begin(115200);
  reg = DataRegister(num_buffer, num_regs, buffer_array, size_array, index_array);
  ser_slave.set_register(reg);

  roll      = (float*)        reg.link(0);  *roll      = 0;
  pitch     = (float*)        reg.link(1);  *pitch     = 0;
  yaw       = (float*)        reg.link(2);  *yaw       = 0;
  loop_time = (unsigned int*) reg.link(3);  *loop_time = 10;
  rax       = (int*)          reg.link(4);  *rax       = 0;
  ray       = (int*)          reg.link(5);  *ray       = 0;
  raz       = (int*)          reg.link(6);  *raz       = 0;
  rgx       = (int*)          reg.link(7);  *rgx       = 0;
  rgy       = (int*)          reg.link(8);  *rgy       = 0;
  rgz       = (int*)          reg.link(9);  *rgz       = 0;
  rmx       = (int*)          reg.link(10); *rmx       = 0;
  rmy       = (int*)          reg.link(11); *rmy       = 0;
  rmz       = (int*)          reg.link(12); *rmz       = 0;

  Wire.begin();
  imu_wakeup();
}

void loop() {
  unsigned long t0 = millis();

  // do stuff
  //imu_read();
  //calculate_orientation();

  ser_slave.scan();
  while (millis() < t0 + *loop_time) {
    ser_slave.scan();
  }
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
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 3 * 2);
  *rax = Wire.read() << 8 | Wire.read();
  *ray = Wire.read() << 8 | Wire.read();
  *raz = Wire.read() << 8 | Wire.read();

  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x43);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 3 * 2);
  *rgx = Wire.read() << 8 | Wire.read();
  *rgy = Wire.read() << 8 | Wire.read();
  *rgz = Wire.read() << 8 | Wire.read();

  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x03);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 3 * 2);
  *rmx = Wire.read() << 8 | Wire.read();
  *rmy = Wire.read() << 8 | Wire.read();
  *rmz = Wire.read() << 8 | Wire.read();
}

void calculate_orientation() {
  int min_scale = -32760;
  int max_scale = 32760;
  float acc_x =  lin_map(*rax, min_scale, max_scale, -2,    2,    acc_offset[0]);
  float acc_y =  lin_map(*ray, min_scale, max_scale, -2,    2,    acc_offset[1]);
  float acc_z =  lin_map(*raz, min_scale, max_scale, -2,    2,    acc_offset[2]);
  float gyro_x = lin_map(*rgx, min_scale, max_scale, -250,  250,  gyro_offset[0]);
  float gyro_y = lin_map(*rgy, min_scale, max_scale, -250,  250,  gyro_offset[1]);
  float gyro_z = lin_map(*rgz, min_scale, max_scale, -250,  250,  gyro_offset[2]);
  float mag_x =  lin_map(*rmx, min_scale, max_scale, -4912, 4912, mag_offset[0]);
  float mag_y =  lin_map(*rmy, min_scale, max_scale, -4912, 4912, mag_offset[1]);
  float mag_z =  lin_map(*rmz, min_scale, max_scale, -4912, 4912, mag_offset[2]);

  float acc_roll = atan2(acc_y, acc_z) * 180.0 / PI; // Converted to degrees
  float acc_pitch = atan2(-acc_x, sqrt(pow(acc_y, 2) + pow(acc_z, 2))) * 180.0 / PI;
  float mag_yaw = atan2(mag_y, mag_x) * 180.0 / PI;

  float dt = (float) * loop_time / 1000;
  *roll  = comp_alpha * (*roll  + gyro_x * dt) + (1 - comp_alpha) * acc_roll;
  *pitch = comp_alpha * (*pitch + gyro_y * dt) + (1 - comp_alpha) * acc_pitch;
  *yaw   = comp_alpha * (*yaw   + gyro_z * dt) + (1 - comp_alpha) * mag_yaw;
}

float lin_map(float x, float x0, float x1, float y0, float y1) {
  return (x - x0) * (y1 - y0) / (x1 - x0) + y0;
}

float lin_map(float x, float x0, float x1, float y0, float y1, float offs) {
  return lin_map(x - offs, x0, x1, y0, y1);
}
