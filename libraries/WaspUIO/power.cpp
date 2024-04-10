#include "WaspUIO.h"

/*
 * Power board:
 * 14 (ANALOG1): 3V3 switch
 * 15 (ANALOG2): 5V switch
 * 16 (ANALOG3): 12V switch
 * 17 (ANALOG4): Lead-Acid meassurement switch
 * ANALOG5     : Lead-Acid measurement input line
 *
 * Sensor board (lemming):
 * DIGITAL1: Maxbotix power switch
 * DIGITAL2: I2C power switch
 * DIGITAL5: OneWire power switch
 * DIGITAL7: SDI-12 power switch
 * DIGITAL6: OneWire data line
 * DIGITAL8: SDI-12 data line
 * 1RX:      Maxbotix data
 * SCL/SDA:  I2C data
 *
 * DIGITAL3: Relay switch on/off (DGPS etc...)
 *
 * Iridium9603
 * DIGITAL4: Iridium sleep switch (grey)
 * 2RX:      Iridium RXD (yellow)
 * 2TX:      Iridium TXD (orange)
 * BATTERY:  Iridium Li-Ion (white)
 * GND:      Iridium GND (black)
 */

/*
 * State handling
 */
void WaspUIO::saveState()
{
  pwr_saved_state = pwr_state;
}

void WaspUIO::loadState()
{
  // Shortcut
  if (pwr_saved_state == pwr_state) { return; }

  // Off
  if (! (pwr_saved_state & PWR_MB))    { pwr_mb(0); }
  if (! (pwr_saved_state & PWR_I2C))   { pwr_i2c(0); }
  if (! (pwr_saved_state & PWR_I2C_2)) { pwr_i2c_2(0); }
  if (! (pwr_saved_state & PWR_1WIRE)) { pwr_1wire(0); }
  if (! (pwr_saved_state & PWR_SDI12)) { pwr_sdi12(0); }
  if (! (pwr_saved_state & PWR_3V3))   { pwr_3v3(0); }
  if (! (pwr_saved_state & PWR_5V))    { pwr_5v(0); }
  if (! (pwr_saved_state & PWR_MAIN))  { pwr_main(0); }

  // On
  if (pwr_saved_state & PWR_MAIN)  { pwr_main(1); }
  if (pwr_saved_state & PWR_3V3)   { pwr_3v3(1); }
  if (pwr_saved_state & PWR_5V)    { pwr_5v(1); }
  if (pwr_saved_state & PWR_MB)    { pwr_mb(1); }
  if (pwr_saved_state & PWR_I2C)   { pwr_i2c(1); }
  if (pwr_saved_state & PWR_I2C_2) { pwr_i2c_2(1); }
  if (pwr_saved_state & PWR_1WIRE) { pwr_1wire(1); }
  if (pwr_saved_state & PWR_SDI12) { pwr_sdi12(1); }
}


/*
 * On/Off devices
 */

bool WaspUIO::_setState(uint8_t device, bool new_state)
{
  bool old_state = pwr_state & device;
  if (new_state) { pwr_state |= device; }
  else           { pwr_state &= ~device; }
  return old_state;
}

/*
 * 1. If the pwr_state has not changed does nothing.
 * 2. Switches high/low the given pin, if the pin is zero does nothing.
 * 3. Sets the on/off the pwr_state of the given device.
 * 4. Returns the old pwr_state.
 */
bool WaspUIO::pwr_switch(uint8_t device, uint8_t pin, bool new_state)
{
  bool old_state = pwr_state & device;
  //cr_printf("*** pwr_switch(%d, %d, %d -> %d)\n", device, pin, old_state, new_state);
  if (new_state == old_state) { return old_state; }

  if (pin != 0)
  {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, (new_state) ? HIGH: LOW);
  }
  return _setState(device, new_state);
}

bool WaspUIO::pwr_main(bool new_state)
{
  uint8_t device = PWR_MAIN;
  bool old_state = pwr_state & device;

  if (new_state == old_state) { return old_state; } // noop
  if (new_state) {}                                 // on
  else { pwr_3v3(0); pwr_5v(0); }                   // off
  return pwr_switch(device, 0, new_state);          // switch
}

bool WaspUIO::pwr_3v3(bool new_state)
{
  uint8_t device = PWR_3V3;
  bool old_state = pwr_state & device;

  if (new_state == old_state) { return old_state; }   // noop
  if (new_state) { pwr_main(1); }                     // on
  else                                                // off
  {
    pwr_i2c(0); pwr_i2c_2(0);
    if (boardType == BOARD_NONE) { pwr_mb(0); pwr_1wire(0); }
  }

  if (batteryType == BATTERY_LITHIUM)                 // switch
  {
    setSensorPower(SENS_3V3, new_state ? SENS_ON : SENS_OFF);
  }
  return pwr_switch(device, 0, new_state);
}

bool WaspUIO::pwr_5v(bool new_state)
{
  uint8_t device = PWR_5V;
  bool old_state = pwr_state & device;

  if (new_state == old_state) { return old_state; }   // noop
  if (new_state) { pwr_main(1); }                     // on
  else                                                // off
  {
    pwr_sdi12(0);
    if (boardType == BOARD_LEMMING) { pwr_mb(0); pwr_1wire(0); }
  }

  if (batteryType == BATTERY_LITHIUM)                 // switch
  {
    setSensorPower(SENS_5V, new_state ? SENS_ON : SENS_OFF);
  }
  return pwr_switch(device, 0, new_state);
}


/*
 * Buses: i2c, sdi-12, etc.
 */

bool WaspUIO::pwr_mb(bool new_state)
{
  uint8_t device = PWR_MB;
  uint8_t pin = (boardType == BOARD_LEMMING) ? PIN_POWER_MAXB: 0;
  bool old_state = pwr_state & device;

  if (new_state == old_state) { return old_state; }   // noop
  if (new_state)                                      // on
  {
    if (boardType == BOARD_LEMMING) { pwr_5v(1); }
    else { pwr_3v3(1); }
  }
  else {}                                             // off
  pwr_switch(device, pin, new_state);                 // switch
  if (old_state == 0) { delay(200); }                 // delay
  return old_state;
}

bool WaspUIO::pwr_i2c(bool new_state)
{
  uint8_t device = PWR_I2C;
  uint8_t pin = (boardType == BOARD_LEMMING) ? PIN_POWER_I2C: 0;
  bool old_state = pwr_state & device;

  if (new_state == old_state) { return old_state; }   // noop
  if (new_state) { pwr_3v3(1); }                      // on
  else {}                                             // off
  pwr_switch(device, pin, new_state);                 // switch
  if (old_state == 0) { delay(100); }                 // delay
  return old_state;
}

bool WaspUIO::pwr_i2c_2(bool new_state)
{
  uint8_t device = PWR_I2C_2;
  uint8_t pin = (boardType == BOARD_LEMMING) ? PIN_POWER_I2C_2: 0;
  bool old_state = pwr_state & device;

  if (new_state == old_state) { return old_state; }   // noop
  if (new_state) { pwr_3v3(1); }                      // on
  else {}                                             // off
  pwr_switch(device, pin, new_state);                 // switch
  if (old_state == 0) { delay(100); }                 // delay
  return old_state;
}

bool WaspUIO::pwr_1wire(bool new_state)
{
  uint8_t device = PWR_1WIRE;
  uint8_t pin = (boardType == BOARD_LEMMING) ? PIN_POWER_1WIRE: 0;
  bool old_state = pwr_state & device;

  if (new_state == old_state) { return old_state; }   // noop
  if (new_state)                                      // on
  {
    if (boardType == BOARD_LEMMING) { pwr_5v(1); }
    else { pwr_3v3(1); }
  }
  else {}                                             // off
  pwr_switch(device, pin, new_state);                 // switch
  if (old_state == 0) { delay(100); }                 // delay
  return old_state;
}

bool WaspUIO::pwr_sdi12(bool new_state)
{
  uint8_t device = PWR_SDI12;
  uint8_t pin = (boardType == BOARD_LEMMING) ? PIN_POWER_SDI12: 0;
  bool old_state = pwr_state & device;

  if (new_state == old_state) { return old_state; }   // noop
  if (new_state) { pwr_5v(1); }                       // on
  else {}                                             // off
  pwr_switch(device, pin, new_state);                 // switch
  if (old_state == 0) { delay(500); }                 // delay
  return old_state;
}


/*
 * Battery
 */

void WaspUIO::readBattery()
{
  // Read battery level (%) or volts, depending on type
  if (batteryType == BATTERY_REG3V3)
  {
    batteryVolts = PWR.getBatteryVolts();
    // force waspmote to think battery is always high (as voltage will always be 3V3)
    batteryLevel = 100;
  }
  else // BATTERY_LITHIUM
  {
    batteryLevel = PWR.getBatteryLevel();
  }

  if      (batteryLevel < 15) { battery = BATTERY_BOTTOM; }
  else if (batteryLevel < 35) { battery = BATTERY_LOW; }
  else if (batteryLevel < 70) { battery = BATTERY_MIDDLE; }
  else                        { battery = BATTERY_HIGH; }

  // cooldown factor
  cooldown = (battery > BATTERY_LOW) ? 1 : 4;
}


/*
 * This is derived from upstream WaspPWR::setSensorPower
 * Power sensors when using lithium battery
 */
void WaspUIO::setSensorPower(uint8_t type, uint8_t mode)
{
//	cr_printf("*** WaspUIO::setSensorPower(%s, %c)\n",
//		(type == SENS_3V3) ? "3V" : (type == SENS_5V) ? "5V" : "?",
//		(mode == SENS_ON) ? '+' : (mode == SENS_OFF) ? '-' : '?'
//	);
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
