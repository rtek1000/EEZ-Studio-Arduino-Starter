#include "CST820.h"

CST820::CST820(int8_t sda_pin, int8_t scl_pin, int8_t rst_pin, int8_t int_pin) {
  _sda = sda_pin;
  _scl = scl_pin;
  _rst = rst_pin;
  _int = int_pin;
}

void CST820::begin(void) {
  // Initialize I2C
  if (_sda != -1 && _scl != -1) {
    Wire.begin(_sda, _scl);
  } else {
    Wire.begin();
  }

#ifndef HIDE_CST820_WARNING
#warning "The CST820 can operate from 10 kHz to 400 kHz, default is 100 kHz"
#warning "Wire.setClock(400000); // set I2C speed, add it after touch.begin() to speed up (and add #include <Wire> at the top of the sketch)"
#endif

  // Int Pin Configuration
  if (_int != -1) {
    pinMode(_int, INPUT_PULLUP);
  }

  // Reset Pin Configuration
  if (_rst != -1) {
    pinMode(_rst, OUTPUT);
    digitalWrite(_rst, LOW);
    delay(10);
    digitalWrite(_rst, HIGH);
    delay(300);
  }

  // Initialize Touch
  i2c_write(0xFE, 0XFF);  // Disable automatic entry into low power mode.
}

bool CST820::getTouch(uint16_t *x, uint16_t *y, uint8_t *gesture) {
  bool FingerIndex = false;
  FingerIndex = (bool)i2c_read(0x02);

  *gesture = i2c_read(0x01);
  if (!(*gesture == SlideUp || *gesture == SlideDown
        || *gesture == SlideLeft || *gesture == SlideRight
        || *gesture == SingleTap || *gesture == DoubleTap
        || *gesture == LongPress)) {
    *gesture = None;
  }

  uint8_t data[4] = { 0 };
  uint8_t data2[4] = { 0 };

  bool _valid_data = false;

  /* False touch simple filter (You can use I2C speed 400kHz to speed up) */
  /* Trying to compensate for missing INT pin - Missing information to configure the INT pin and be able to use it */
  for (uint8_t i = 0; i < 5; i++) {
    i2c_read_continuous(0x03, data, 4);
    delay(2); // Give the CST820 some time to process
    i2c_read_continuous(0x03, data2, 4);

    if ((data[0] == data2[0]) && (data[1] == data2[1]) && (data[2] == data2[2]) && (data[3] == data2[3])) {
      *x = ((data[0] & 0x0f) << 8) | data[1];
      *y = ((data[2] & 0x0f) << 8) | data[3];

      _valid_data = true;

      break;
    }
  }

  if (_valid_data) {
    return FingerIndex;
  } else {
    return false;
  }
}

uint8_t CST820::i2c_read(uint8_t addr) {
  uint8_t rdData;
  uint8_t rdDataCount;
  do {
    Wire.beginTransmission(I2C_ADDR_CST820);
    Wire.write(addr);
    Wire.endTransmission(false);  // Restart
    rdDataCount = Wire.requestFrom(I2C_ADDR_CST820, 1);
  } while (rdDataCount == 0);
  while (Wire.available()) {
    rdData = Wire.read();
  }
  return rdData;
}

uint8_t CST820::i2c_read_continuous(uint8_t addr, uint8_t *data, uint32_t length) {
  Wire.beginTransmission(I2C_ADDR_CST820);
  Wire.write(addr);
  if (Wire.endTransmission(true)) return -1;
  Wire.requestFrom(I2C_ADDR_CST820, length);
  for (int i = 0; i < length; i++) {
    *data++ = Wire.read();
  }
  return 0;
}

void CST820::i2c_write(uint8_t addr, uint8_t data) {
  Wire.beginTransmission(I2C_ADDR_CST820);
  Wire.write(addr);
  Wire.write(data);
  Wire.endTransmission();
}

uint8_t CST820::i2c_write_continuous(uint8_t addr, const uint8_t *data, uint32_t length) {
  Wire.beginTransmission(I2C_ADDR_CST820);
  Wire.write(addr);
  for (int i = 0; i < length; i++) {
    Wire.write(*data++);
  }
  if (Wire.endTransmission(true)) return -1;
  return 0;
}