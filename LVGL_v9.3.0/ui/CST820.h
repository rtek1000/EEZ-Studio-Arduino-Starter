#ifndef _CST820_H
#define _CST820_H

#include <Wire.h>

#define I2C_ADDR_CST820 0x15

/* Uncomment these lines to hide the warning about I2C speed: */
// #ifndef HIDE_CST820_WARNING
// #define HIDE_CST820_WARNING
// #endif

#ifndef HIDE_CST820_WARNING
#warning "Uncomment these lines above to hide the warning about I2C speed"
#endif

//gesture
enum GESTURE {
  None = 0x00,        //No gestures
  SlideDown = 0x01,   //Scroll down
  SlideUp = 0x02,     //Swipe up
  SlideLeft = 0x03,   //Swipe left
  SlideRight = 0x04,  //Swipe right
  SingleTap = 0x05,   //Click
  DoubleTap = 0x0B,   //Double click
  LongPress = 0x0C    //Long press
};

/**************************************************************************/
/*!
    @brief  CST820 I2C CTP controller driver
*/
/**************************************************************************/
class CST820 {
public:
  CST820(int8_t sda_pin = -1, int8_t scl_pin = -1, int8_t rst_pin = -1, int8_t int_pin = -1);

  void begin(void);
  bool getTouch(uint16_t *x, uint16_t *y, uint8_t *gesture);

private:
  int8_t _sda, _scl, _rst, _int;

  uint8_t i2c_read(uint8_t addr);
  uint8_t i2c_read_continuous(uint8_t addr, uint8_t *data, uint32_t length);
  void i2c_write(uint8_t addr, uint8_t data);
  uint8_t i2c_write_continuous(uint8_t addr, const uint8_t *data, uint32_t length);
};
#endif