#include "WaspUIO.h"

/*
 * In this file we override fuctions from Libellium's api library.
 */


/*
 * RTC uses I2C. If there are other sensors with I2C they must be powered on
 * when using RTC, otherwise it won't work.
 *
 * TODO Optimize, check configuration to switch on 3V3 only if there're I2C
 * devices. Consider I2C isolators as well.
 *
 */

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


/*
 * The upstream code is written in such a way that it sends the mote name (id)
 * with every frame. But we only want to send it in uptime frame.
 *
 * Upstream code is also very inefficient, it reads and writes the EEPROM too
 * much.
 */
void WaspFrame::setID(char* moteID)
{
  char c;

  // clear the waspmote ID attribute
  memset( _waspmoteID, 0x00, sizeof(_waspmoteID) );

  // set the mote ID from EEPROM memory
  for (int i=0 ; i < 16 ; i++)
  {
    c = moteID[i];
    if (c == '#' || c == ' ') { c = '_'; }
    _waspmoteID[i] = c;
    if (c == '\0') { break; } // break if end of string
  }
}

void WaspFrame::getID(char* moteID)
{
  memset( _waspmoteID, 0x00, sizeof(_waspmoteID) );
}
