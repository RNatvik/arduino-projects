#include "RACProtocol.h"
#include "Wire.h"
//----------------------------
//  DONE:
//        Made Timer2 custom with clock period 0.0005s (500us)
//        Created ISR for driving stepper motor pins
//        Added DRIVE variables to SERIAL register
//        Enable writing from sketch to register
//  TODO:
//        Implement IMU Connection
//        Forward IMU data to SERIAL register
//----------------------------



//----------------------------
//  ISR
//----------------------------
const uint16_t T2_LOAD = 0;
const uint16_t T2_COMP = 10 - 1; // 50 kHz (20 us) timer with prescaler 32

//----------------------------
//  GENERAL SETTINGS
//----------------------------
unsigned int transmit_rate = 500; // 50 000 Hz / 500 = 100 Hz transmit frequency
unsigned int transmit_counter = 0;
bool transmit = false;

//----------------------------
//  DRIVE
//----------------------------
const int drive_pin1 = 11;
const int drive_pin2 = 9;
const int dir_pin1 = 12;
const int dir_pin2 = 10;

int drive_delay1 = 1000;
int drive_delay2 = 1000;
unsigned long drive_counter1 = 0;
unsigned long drive_counter2 = 0;
long step_counter1 = 0;
long step_counter2 = 0;
bool drive_enable1 = false;
bool drive_enable2 = false;
bool drive_state1 = false;
bool drive_state2 = false;
bool dir_state1 = false;
bool dir_state2 = false;

//----------------------------
//  SENSOR
//----------------------------
const int MPU_ADDR = 0x68;
int acc_x, acc_y, acc_z = 0;
int gyro_x, gyro_y, gyro_z = 0;

//----------------------------
//  SERIAL
//----------------------------
RACProtocol protocol;

// Protocol variables
const int num_registers = 15;
const int buffer_size = 30;
int register_size[] = {2, 2, 1, 1, 1, 1, 4, 4, 2, 2, 2, 2, 2, 2, 2};
int index_array[num_registers];
byte value_buffer[buffer_size];

//----------------------------
//  REGISTERS
//  Specify which registers (<type> *regX = &my_var;) correspond to which program variables
//----------------------------
int *reg0 = &drive_delay1;
int *reg1 = &drive_delay2;
bool *reg2 = &drive_enable1;
bool *reg3 = &drive_enable2;
bool *reg4 = &dir_state1;
bool *reg5 = &dir_state2;
long *reg6 = &step_counter1;
long *reg7 = &step_counter2;
unsigned int *reg8 = &transmit_rate;
int *reg9 = &acc_x;
int *reg10 = &acc_y;
int *reg11 = &acc_z;
int *reg12 = &gyro_x;
int *reg13 = &gyro_y;
int *reg14 = &gyro_z;

int transmit_num = 8;
int transmit_registers[] = {6, 7, 9, 10, 11, 12, 13, 14};

//----------------------------
//  TEST
//  variables used only for testing purposes
//----------------------------


//----------------------------
//  SETUP
//----------------------------
void setup() {
  
  Serial.begin(250000);
  protocol = RACProtocol(buffer_size, value_buffer, num_registers, register_size, index_array);
  Wire.begin();
  imu_wakeup();

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
  imu_read();
  bool updated_registers[num_registers];
  memset(updated_registers, 0, num_registers);
  bool new_values = protocol.receive_lite(updated_registers);
  update_values(updated_registers);
  if (transmit) {
    transmit = false;
    protocol.transmit(transmit_registers, transmit_num);
  }

}

//----------------------------
//  INTERUPT SERVICE ROUTINE
//----------------------------
ISR(TIMER2_COMPA_vect) {
  TCNT2 = T2_LOAD;
  if (drive_enable1) {
    drive_counter1 += 1;
    if (drive_counter1 >= drive_delay1) {
      drive_counter1 = 0;
      drive_state1 = not drive_state1;
      digitalWrite(dir_pin1, dir_state1);
      digitalWrite(drive_pin1, drive_state1);
      step_counter1 += 1; // NOTE: This variable must be either incremented or decremented based on direction
    }
  }
  if (drive_enable2) {
    drive_counter2 += 1;
    if (drive_counter2 >= drive_delay2) {
      drive_counter2 = 0;
      drive_state2 = not drive_state2;
      digitalWrite(dir_pin2, dir_state2);
      digitalWrite(drive_pin2, drive_state2);
      step_counter2 += 1; // NOTE: This variable must be either incremented or decremented based on direction
    }
  }
  transmit_counter++;
  if (transmit_counter >= transmit_rate) {
    transmit_counter = 0;
    transmit = true;
  }
}

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
  acc_x = Wire.read() << 8 | Wire.read();
  acc_y = Wire.read() << 8 | Wire.read();
  acc_z = Wire.read() << 8 | Wire.read();

  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x43);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 3 * 2);
  gyro_x = Wire.read() << 8 | Wire.read();
  gyro_y = Wire.read() << 8 | Wire.read();
  gyro_z = Wire.read() << 8 | Wire.read();
}

//----------------------------
//  UPDATE VALUES FROM REGISTER
//----------------------------
void update_values(bool updated_registers[]) {
  for (int i = 0; i < num_registers; i++) {
    bool updated = updated_registers[i];
    if (updated) {
      retrieve_register(i);
      updated_registers[i] = false;
    } else {
      put_register(i);
    }
  }
}

//----------------------------
//  Write value to register
//----------------------------
void put_register(int reg_num) {
  switch (reg_num) {
    case 0:
      protocol.write_int(0, *reg0);
      break;
    case 1:
      protocol.write_int(1, *reg1);
      break;
    case 2:
      protocol.write_bool(2, *reg2);
      break;
    case 3:
      protocol.write_bool(3, *reg3);
      break;
    case 4:
      protocol.write_bool(4, *reg4);
      break;
    case 5:
      protocol.write_bool(5, *reg5);
      break;
    case 6:
      protocol.write_long(6, *reg6);
      break;
    case 7:
      protocol.write_long(7, *reg7);
      break;
    case 8:
      protocol.write_uint(8, *reg8);
      break;
    case 9:
      protocol.write_int(9, *reg9);
      break;
    case 10:
      protocol.write_int(10, *reg10);
      break;
    case 11:
      protocol.write_int(11, *reg11);
      break;
    case 12:
      protocol.write_int(12, *reg12);
      break;
    case 13:
      protocol.write_int(13, *reg13);
      break;
    case 14:
      protocol.write_int(14, *reg14);
      break;
  }
}

void retrieve_register(int reg_num) {
  switch (reg_num) {
    case 0:
      *reg0 = protocol.get_int(0);
      break;
    case 1:
      *reg1 = protocol.get_int(1);
      break;
    case 2:
      *reg2 = protocol.get_bool(2);
      break;
    case 3:
      *reg3 = protocol.get_bool(3);
      break;
    case 4:
      *reg4 = protocol.get_bool(4);
      break;
    case 5:
      *reg5 = protocol.get_bool(5);
      break;
    case 6:
      *reg6 = protocol.get_long(6);
      break;
    case 7:
      *reg7 = protocol.get_long(7);
      break;
    case 8:
      *reg8 = protocol.get_uint(8);
      break;
    case 9:
      *reg9 = protocol.get_int(9);
      break;
    case 10:
      *reg10 = protocol.get_int(10);
      break;
    case 11:
      *reg11 = protocol.get_int(11);
      break;
    case 12:
      *reg12 = protocol.get_int(12);
      break;
    case 13:
      *reg13 = protocol.get_int(13);
      break;
    case 14:
      *reg14 = protocol.get_int(14);
      break;
  }
}
