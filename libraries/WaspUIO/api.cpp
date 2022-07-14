#include "WaspUIO.h"

/*
 * In this file we override fuctions from Libellium's api library.
 */

/*
 * We don't need to call RTC to know the time
 */
void WaspUIO::dateTime(uint16_t* date, uint16_t* time)
{
  timestamp_t ts;
  uint32_t epoch = UIO.getEpochTime();

  RTC.breakTimeAbsolute(epoch, &ts);
  *date = FAT_DATE(ts.year+2000, ts.month, ts.date);
  *time = FAT_TIME(ts.hour, ts.minute, ts.second);
}

void WaspSD::setFileDate() {}


/*
 * The upstream code is written in such a way that it sends the mote name (id)
 * with every frame. But we only want to send it in uptime frame.
 *
 * Upstream code is also very inefficient, it reads and writes the EEPROM too
 * much.
 */
void WaspFrame::setID(char* moteID)
{
  // clear the waspmote ID attribute
  memset( _waspmoteID, 0x00, sizeof(_waspmoteID) );

  // set the mote ID from EEPROM memory
  for (int i=0 ; i < 16 ; i++)
  {
    char c = moteID[i];
    if (c == '#' || c == ' ') { c = '_'; }
    _waspmoteID[i] = c;
    if (c == '\0') { break; } // break if end of string
  }
}

void WaspFrame::getID(char* moteID)
{
  memset( _waspmoteID, 0x00, sizeof(_waspmoteID) );
}


/*
 * We have a number of I2C sensors (without isolators) different than those
 * from upstream waspmote.
 */

uint8_t WaspI2C::scanSlaves()
{
  uint8_t addresses[] = {
    I2C_ADDRESS_Lemming_BME280,
    I2C_ADDRESS_LAGOPUS_BME280,
    I2C_ADDRESS_TMP117, // TMP102 has the same address
    I2C_ADDRESS_LAGOPUS_VL53L1X,
    I2C_ADDRESS_LAGOPUS_MLX90614,
    I2C_ADDRESS_LAGOPUS_AS726X,
    I2C_ADDRESS_SHT31,
  };

  _slavePresent = false;
  for (uint8_t i=0; i < sizeof addresses; i++)
  {
    uint8_t status = scan(addresses[i]);
    //cr_printf("scanSlaves(): %02x = %hhu\n", addresses[i], status);
    if (status == 0)
    {
      _slavePresent = true;
      break;
    }
  }

  return _slavePresent;
}

/*
 * Override PWR.setSensorPower to support power and sensor boards.
 */

bool WaspPWR::setSensorPower(uint8_t type, uint8_t mode)
{
//  cr_printf("*** WaspPWR::setSensorPower(%s, %c)\n",
//      (type == SENS_3V3) ? "3V"
//      : (type == SENS_5V) ? "5V"
//      : (type == SENS_I2C) ? "i2c"
//      : (type == SENS_ALL) ? "all"
//      : "?",
//      (mode == SENS_ON) ? '+' : (mode == SENS_OFF) ? '-' : '?'
//  );
    bool new_state = (mode == SENS_ON)? true: false;

    if (type == SENS_3V3) { return UIO.pwr_3v3(new_state); }  // SENS_3V3 = 0
    if (type == SENS_5V)  { return UIO.pwr_5v(new_state); }   // SENS_5V  = 1
    if (type == SENS_I2C) { return UIO.pwr_i2c(new_state); }  // SENS_I2C = 2
    if (type == SENS_ALL) { return UIO.pwr_main(new_state); } // SENS_ALL = 3

    // XXX Should never reach here
    return false;
}
