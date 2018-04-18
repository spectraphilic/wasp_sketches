#include "WaspUIO.h"

// RTC uses I2C. If there are other sensors with I2C they must be powered on
// when using RTC, otherwise it won't work.

// TODO Optimize, check configuration to switch on 3V3 only if there're I2C
// devices. Consider I2C isolators as well.

void TwoWire::secureBegin()
{
  // Do we need to switch on 5V as well?
  _3V3_ON = UIO.v33(1);
  if (! _3V3_ON)
  {
    delay(50);
  }
}


void TwoWire::secureEnd()
{
  UIO.v33(_3V3_ON);
}
