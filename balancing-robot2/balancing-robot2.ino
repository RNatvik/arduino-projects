#include "RACProtocol.h"
#include "Wire.h"

//----------------------------
//  SERIAL
//----------------------------
RACProtocol protocol;

// Protocol variables
const int num_registers = 8;
const int buffer_size = 28 + 2; // 7x float + uint
int register_size[] = {4, 4, 4, 4, 4, 4, 4, 2};
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
unsigned int * dt;

int transmit_num = 4;
int transmit_registers[] = {4, 5, 6, 7};

//----------------------------
//  ISR
//----------------------------
const uint16_t T2_LOAD = 0;
const uint16_t T2_COMP = 10 - 1; // 50 kHz (20 us) timer with prescaler 32

//----------------------------
//  GENERAL SETTINGS
//----------------------------
unsigned int loop_time = 4; // ms
float comp_alpha = 0.98; // Complementary filter alpha
float acc_offset[] = { -0.0023, -0.0114, -0.0337};
float gyro_offset[] = { -3.8794, -0.1854, 0.2284};
int acc_scale = 16384;
int gyro_scale = 131;

//----------------------------
//  DRIVE
//----------------------------
const int STEP_PIN1 = 11; // Drive pin 1
const int STEP_PIN2 = 9;  // Drive pin 2
const int DIR_PIN1 = 12;  // Direction pin 1
const int DIR_PIN2 = 10;  // Direction pin 2

int drive_delay1 = 1000;
int drive_delay2 = 1000;
unsigned long drive_counter1 = 0;
unsigned long drive_counter2 = 0;

bool drive_enable = false;
bool drive_state1 = false;
bool drive_state2 = false;
bool dir_state1 = false;
bool dir_state2 = false;

//----------------------------
//  SENSOR
//----------------------------
const int MPU_ADDR = 0x68;
float acc_x, acc_y, acc_z = 0;
float gyro_x, gyro_y, gyro_z = 0;

//----------------------------
//  REGULATOR
//----------------------------
bool regulator_first = true;
float error_deadband = 0.1;
float output_deadband = 5;
float e0 = 0;
float esum = 0;
float olim = 800;


//----------------------------
//  SETUP
//----------------------------
void setup() {
  // --- Start serial protocol ---
  Serial.begin(250000);
  protocol = RACProtocol(buffer_size, value_buffer, num_registers, register_size, index_array);
  // Assign serial registers and set initial values. "index_array" has been calculated by construction of RACProtocol
  kp     = (float*)        &value_buffer[index_array[0]]; *kp     = 0;
  ki     = (float*)        &value_buffer[index_array[1]]; *ki     = 0;
  kd     = (float*)        &value_buffer[index_array[2]]; *kd     = 0;
  target = (float*)        &value_buffer[index_array[3]]; *target = 0;
  roll   = (float*)        &value_buffer[index_array[4]]; *roll   = 0;
  pitch  = (float*)        &value_buffer[index_array[5]]; *pitch  = 0;
  yaw    = (float*)        &value_buffer[index_array[6]]; *yaw    = 0;
  dt     = (unsigned int*) &value_buffer[index_array[7]]; *dt     = loop_time;


  // --- Initialize IMU ---
  Wire.begin();
  imu_wakeup();

  // --- Create interrupt timer ---
  TCCR2A = 0;
  // Set Timer2 Prescaler to 32 (0-1-1)
  TCCR2B &= ~(1 << CS22);
  TCCR2B |= (1 << CS21);
  TCCR2B |= (1 << CS20);

  TCNT2 = T2_LOAD;
  OCR2A = T2_COMP;

  TIMSK2 = (1 << OCIE2A);

  sei();
}

//----------------------------
//  LOOP
//----------------------------
void loop() {
  unsigned long t0 = millis();

  bool updated_registers[num_registers];
  memset(updated_registers, 0, num_registers);
  protocol.receive_lite(updated_registers); // Read incomming serial

  // Calculate orientation
  imu_read(); // Read IMU
  float acc_roll = atan2(acc_y, acc_z) * 180 / PI; // Converted to degrees
  float acc_pitch = atan2(-acc_x, sqrt(pow(acc_y, 2) + pow(acc_z, 2))) * 180 / PI;
  *roll = comp_alpha * (*roll + gyro_x * (float)loop_time / 1000) + (1 - comp_alpha) * acc_roll;
  *pitch = comp_alpha * (*pitch + gyro_y * (float)loop_time / 1000) + (1 - comp_alpha) * acc_pitch;
  *yaw += gyro_z * (float)loop_time / 1000;

  // Run regulator
  pid(*target, *pitch);

  // Transmit state
  // protocol.transmit(transmit_registers, transmit_num);

  while (millis() < t0 + loop_time) {
    // Do nothing
  }
  *dt = millis() - t0;
}

//----------------------------
//  INTERUPT SERVICE ROUTINE
//----------------------------
ISR(TIMER2_COMPA_vect) {
  TCNT2 = T2_LOAD;
  if (drive_enable) {
    drive_counter1 += 1;
    if (drive_counter1 >= drive_delay1) {
      drive_counter1 = 0;
      drive_state1 = not drive_state1;
      digitalWrite(DIR_PIN1, dir_state1);
      digitalWrite(STEP_PIN1, drive_state1);
    }
    drive_counter2 += 1;
    if (drive_counter2 >= drive_delay2) {
      drive_counter2 = 0;
      drive_state2 = not drive_state2;
      digitalWrite(DIR_PIN2, dir_state2);
      digitalWrite(STEP_PIN2, drive_state2);
    }
  }
}

//----------------------------
//  TEST REGULATOR ------TEST TEST TEST-------
//----------------------------
void regulate(float pitch) {
  if (abs(pitch) < 15) {
    float error = 0 - pitch;
    float output = 1000;
    if (abs(error) > 0.5) {
      output = 100 / (*kp * error);
      drive_delay1 = abs((int)output);
      drive_delay2 = abs((int)output);

      drive_enable = true;
      dir_state1 = ((int)output) <= 0;
      dir_state2 = ((int)output) >= 0;
    } else {
      drive_enable = false;
    }
  } else {
    drive_enable = false;
  }
  //  Serial.println();
  //  Serial.print("Error: "); Serial.print(error); Serial.print(",");
  //  Serial.print("Output: "); Serial.print(output); Serial.print(",");
  //  Serial.print("DD: "); Serial.print(drive_delay1); Serial.print(",");
  //  Serial.print("DE: "); Serial.print(drive_enable); Serial.print(",");
  //  Serial.print("DS1: "); Serial.print(dir_state1); Serial.print(",");
  //  Serial.print("DS2: "); Serial.print(dir_state2); Serial.println();
}

void pid(float target, float pitch) {
  if (abs(pitch) < 15) {
    float t = ((float) * dt) / 1000;
    float elim = olim / (*ki * t);

    float e = target - pitch;
    if (abs(e) < error_deadband) {
      e = 0;
    }

    float de = e - e0;
    e0 = e;
    esum = constrain(esum + e, -elim, elim);
    if (regulator_first) {
      regulator_first = false;
   } else {
      float p = (*kp) * e;
      float i = (*ki) * esum * t;
      float d = (*kd) * de / t;

      float output = constrain(p + i + d, -olim, olim);
      reg(output);
    }
  } else {
    drive_enable = false;
  }
}

void reg(float output) {
  if (abs(output) > output_deadband) {
    output = 10000 / output;
    drive_delay1 = abs((int)output);
    drive_delay2 = abs((int)output);

    drive_enable = true;
    dir_state1 = ((int)output) <= 0;
    dir_state2 = ((int)output) >= 0;
  } else {
    drive_enable = false;
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
  int raw_acc_x = Wire.read() << 8 | Wire.read();
  int raw_acc_y = Wire.read() << 8 | Wire.read();
  int raw_acc_z = Wire.read() << 8 | Wire.read();

  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x43);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 3 * 2);
  int raw_gyro_x = Wire.read() << 8 | Wire.read();
  int raw_gyro_y = Wire.read() << 8 | Wire.read();
  int raw_gyro_z = Wire.read() << 8 | Wire.read();

  acc_x = (float)raw_acc_x / acc_scale - acc_offset[0];
  acc_y = (float)raw_acc_y / acc_scale - acc_offset[1];
  acc_z = (float)raw_acc_z / acc_scale - acc_offset[2];
  gyro_x = (float)raw_gyro_x / gyro_scale - gyro_offset[0];
  gyro_y = (float)raw_gyro_y / gyro_scale - gyro_offset[1];
  gyro_z = (float)raw_gyro_z / gyro_scale - gyro_offset[2];

}
