#include "WaspUIO.h"

/*
 * In this file we override fuctions from Libellium's api library.
 */


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
  _slavePresent = (
    !scan(I2C_ADDRESS_Lemming_BME280) ||
    !scan(I2C_ADDRESS_LAGOPUS_BME280) ||
    !scan(I2C_ADDRESS_LAGOPUS_TMP102) ||
    !scan(I2C_ADDRESS_LAGOPUS_VL53L1X) ||
    !scan(I2C_ADDRESS_LAGOPUS_MLX90614) ||
    !scan(I2C_ADDRESS_LAGOPUS_AS726X)
  );
  cr.println(F("_slavePresent=%d"), _slavePresent);
  return _slavePresent;
}


/*
 * Override PWR.setSensorPower to support power and sensor boards.
 */

void WaspPWR::setSensorPower(uint8_t type, uint8_t mode)
{
  bool new_state = (mode == SENS_ON)? true: false;

  if (type == SENS_3V3)
  {
    UIO.v33(new_state);
  }
  else if (type == SENS_5V)
  {
    UIO.v5(new_state);
  }
}

/*
 * This is copy/paste from upstream WaspPWR::setSensorPower
*/
void WaspUIO::setSensorPower(uint8_t type, uint8_t mode)
{
	pinMode(SENS_PW_3V3,OUTPUT);
	pinMode(SENS_PW_5V,OUTPUT);
	
	switch (type)
	{
		case SENS_3V3: 	
						if (mode == SENS_ON) 
						{
							WaspRegister |= REG_3V3;
							digitalWrite(SENS_PW_3V3,HIGH);
						}
						else if (mode == SENS_OFF) 
						{
							WaspRegister &= ~REG_3V3;
							digitalWrite(SENS_PW_3V3,LOW);
							
						}						
						break;
						
		case SENS_5V:	
						if (mode == SENS_ON) 
						{
							WaspRegister |= REG_5V;
							digitalWrite(SENS_PW_5V,HIGH);
							delay(1);
						}
						else if (mode == SENS_OFF) 
						{
							WaspRegister &= ~REG_5V;
							digitalWrite(SENS_PW_5V,LOW);
						}
						break;
						
		default:		break;
	}
}
