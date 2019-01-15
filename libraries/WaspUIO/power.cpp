#include "WaspUIO.h"

/*
 * Power board:
 * 14     : 3V3 switch
 * 15     : 5V switch
 * 16     : 12V switch
 * 17     : Lead-Acid meassurement switch
 * ANALOG5: Lead-Acid measurement input line
 *
 * Sensor board (lemming):
 * DIGITAL1: Maxbotix power switch
 * DIGITAL2: I2C power switch
 * DIGITAL5: OneWire power switch
 * DIGITAL7: SDI-12 power switch
 *
 * DIGITAL6: OneWire data line
 * DIGITAL8: SDI-12 data line
 * : 1RX Maxbotix
 * : SCL/SDA I2C
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
  if (! (pwr_saved_state & PWR_1WIRE)) { pwr_1wire(0); }
  if (! (pwr_saved_state & PWR_SDI12)) { pwr_sdi12(0); }
  if (! (pwr_saved_state & PWR_3V3))   { pwr_3v3(0); }
  if (! (pwr_saved_state & PWR_5V))    { pwr_5v(0); }
  if (! (pwr_saved_state & PWR_LEAD_VOLTAGE)) { pwr_leadVoltage(0); }
  if (! (pwr_saved_state & PWR_MAIN))  { pwr_main(0); }

  // On
  if (pwr_saved_state & PWR_MAIN)  { pwr_main(1); }
  if (pwr_saved_state & PWR_3V3)   { pwr_3v3(1); }
  if (pwr_saved_state & PWR_5V)    { pwr_5v(1); }
  if (pwr_saved_state & PWR_LEAD_VOLTAGE) { pwr_leadVoltage(1); }
  if (pwr_saved_state & PWR_MB)    { pwr_mb(1); }
  if (pwr_saved_state & PWR_I2C)   { pwr_i2c(1); }
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
 * 1. If the pwr_state has not changed does nothgin.
 * 2. Switches high/low the given pin, if the pin is zero does nothing.
 * 3. Sets the on/off the pwr_state of the given device.
 * 4. Returns the old pwr_state.
 */
bool WaspUIO::pwr_switch(uint8_t device, uint8_t pin, bool new_state)
{
  bool old_state = pwr_state & device;
  //cr.println(F("*** pwr_switch(%d, %d, %d -> %d)"), device, pin, old_state, new_state);
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
  uint8_t pin = (batteryType == BATTERY_LEAD) ? 16: 0;
  bool old_state = pwr_state & device;

  if (new_state == old_state) { return old_state; }   // noop
  if (new_state) {}                                   // on
  else { pwr_3v3(0); pwr_5v(0); pwr_leadVoltage(0); } // off
  return pwr_switch(device, pin, new_state);          // switch
}

bool WaspUIO::pwr_3v3(bool new_state)
{
  uint8_t device = PWR_3V3;
  uint8_t pin = (batteryType == BATTERY_LEAD) ? 14: 0;
  bool old_state = pwr_state & device;

  if (new_state == old_state) { return old_state; }   // noop
  if (new_state) { pwr_main(1); }                     // on
  else                                                // off
  {
    pwr_i2c(0);
    if (boardType == BOARD_NONE) { pwr_mb(0); pwr_1wire(0); }
  }

  if (batteryType == BATTERY_LITHIUM)                 // switch
  {
    setSensorPower(SENS_3V3, new_state ? SENS_ON : SENS_OFF);
  }
  return pwr_switch(device, pin, new_state);
}

bool WaspUIO::pwr_5v(bool new_state)
{
  uint8_t device = PWR_5V;
  uint8_t pin = (batteryType == BATTERY_LEAD) ? 15: 0;
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
  return pwr_switch(device, pin, new_state);
}

bool WaspUIO::pwr_leadVoltage(bool new_state)
{
  uint8_t device = PWR_LEAD_VOLTAGE;
  uint8_t pin = (batteryType == BATTERY_LEAD) ? 17: 0;
  bool old_state = pwr_state & device;

  if (new_state == old_state) { return old_state; }   // noop
  if (new_state) { pwr_main(1); }                     // on
  else {}                                             // off
  return pwr_switch(device, pin, new_state);          // switch
}


/*
 * Buses: i2c, sdi-12, etc.
 */

bool WaspUIO::pwr_mb(bool new_state)
{
  uint8_t device = PWR_MB;
  uint8_t pin = (boardType == BOARD_LEMMING) ? DIGITAL1: 0;
  bool old_state = pwr_state & device;

  if (new_state == old_state) { return old_state; }   // noop
  if (new_state)                                      // on
  {
    if (boardType == BOARD_LEMMING) { pwr_5v(1); }
    else { pwr_3v3(1); }
  }
  else {}                                             // off
  return pwr_switch(device, pin, new_state);          // switch
}

bool WaspUIO::pwr_i2c(bool new_state)
{
  uint8_t device = PWR_I2C;
  uint8_t pin = (boardType == BOARD_LEMMING) ? DIGITAL2: 0;
  bool old_state = pwr_state & device;

  if (new_state == old_state) { return old_state; }   // noop
  if (new_state) { pwr_3v3(1); }                      // on
  else {}                                             // off
  return pwr_switch(device, pin, new_state);          // switch
}

bool WaspUIO::pwr_1wire(bool new_state)
{
  uint8_t device = PWR_1WIRE;
  uint8_t pin = (boardType == BOARD_LEMMING) ? DIGITAL5: 0;
  bool old_state = pwr_state & device;

  if (new_state == old_state) { return old_state; }   // noop
  if (new_state)                                      // on
  {
    if (boardType == BOARD_LEMMING) { pwr_5v(1); }
    else { pwr_3v3(1); }
  }
  else {}                                             // off
  return pwr_switch(device, pin, new_state);          // switch
}

bool WaspUIO::pwr_sdi12(bool new_state)
{
  uint8_t device = PWR_SDI12;
  uint8_t pin = (boardType == BOARD_LEMMING) ? DIGITAL7: 0;
  bool old_state = pwr_state & device;

  if (new_state == old_state) { return old_state; }   // noop
  if (new_state)                                      // on
  {
    if (pwr_5v(1) == 0) { delay(500); };
  }
  else {}                                             // off
  return pwr_switch(device, pin, new_state);          // switch
}


/*
 * Battery
 */

void WaspUIO::readBattery()
{
  battery = BATTERY_HIGH;

  // Read battery level (%) or volts, depending on type
  if (batteryType == BATTERY_LITHIUM)
  {
    batteryLevel = PWR.getBatteryLevel();
    if      (batteryLevel <= 35) { battery = BATTERY_LOW; }
    else if (batteryLevel <= 70) { battery = BATTERY_MIDDLE; }
  }
  else
  {
    batteryVolts = getLeadBatteryVolts();
    if      (batteryVolts <= 10.5) { battery = BATTERY_LOW; }
    else if (batteryVolts <= 11.6) { battery = BATTERY_MIDDLE; }
  }

  // cooldown factor
  if (battery == BATTERY_LOW) { cooldown = 3; }
  else if (battery == BATTERY_MIDDLE) { cooldown = 2; }
  else if (battery == BATTERY_HIGH) { cooldown = 1; }
}


float WaspUIO::getBatteryVolts()
{
  if (batteryType == BATTERY_LITHIUM)
  {
    return PWR.getBatteryVolts();
  }

  return getLeadBatteryVolts();
}


float WaspUIO::getLeadBatteryVolts()
{
  int analog5;
  float R1 = 10;  // 10k resistor
  float R2 = 2.2; // 2k2 resistor
  float volts;
  char volts_str[15];

  pinMode(ANALOG5, INPUT);

  if (! pwr_leadVoltage(1)) { delay(100); } // on, let power to stabilize
  analog5 = analogRead(ANALOG5);            // Analog output (0 - 3.3V): from 0 to 1023
  pwr_leadVoltage(0);                       // off

  volts = analog5  * (R1 + R2) / R2 * 3.3 / 1023 ;
  Utils.float2String(volts, volts_str, 2);
  debug(F("Lead acid battery analog5=%d volts=%s"), analog5, volts_str);

  return volts;
}

/*
 * This is derived from upstream WaspPWR::setSensorPower
 * Power sensors when using lithium battery
 */
void WaspUIO::setSensorPower(uint8_t type, uint8_t mode)
{
	//cr.println(F("*** WaspUIO::setSensorPower(%d, %d)"), type, mode);
	pinMode(SENS_PW_3V3,OUTPUT);
	pinMode(SENS_PW_5V,OUTPUT);

	switch (type)
	{
		case SENS_3V3:
						if (mode == SENS_ON)
						{
							WaspRegister |= REG_3V3;
							digitalWrite(SENS_PW_3V3,HIGH);
							if (type == i2c)
							{
								// Ask MLX90614 to switch from PWM to SMBus, set SCL to LOW for more
								// than 1.44ms
								pinMode(I2C_SCL, OUTPUT);
								digitalWrite(I2C_SCL, LOW);
								delay(2);
							}
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
