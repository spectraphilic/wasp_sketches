#include "WaspUIO.h"


void WaspUIO::startPowerBoard()
{
  //debug(F("Start power board"));
  pinMode(14, OUTPUT); // 3V3 switch
  pinMode(15, OUTPUT); // 5V switch
  pinMode(16, OUTPUT); // 12V switch
  pinMode(17, OUTPUT); // Lead-Acid meassurement switch
  pinMode(ANALOG5, INPUT);  // Lead-Acid measurement input line
}

void WaspUIO::startSensorBoard()
{
  //debug(F("Start sensor board"));
  pinMode(DIGITAL1, OUTPUT); // Maxbotix power switch
  pinMode(DIGITAL2, OUTPUT); // I2C power switch
  pinMode(DIGITAL5, OUTPUT); // OneWire power switch
  pinMode(DIGITAL7, OUTPUT); // SDI-12 power switch

  pinMode(DIGITAL6, INPUT); // OneWire data line
  pinMode(DIGITAL8, INPUT); // SDI-12 data line
  // 1RX Maxbotix
  // SCL/SDA I2C
}


/*
 * State handling
 */
void WaspUIO::saveState()
{
  saved_state = state;
  saved_WaspRegister = WaspRegister;
}

void WaspUIO::loadState()
{
  // Shortcut
  if (saved_state == state && saved_WaspRegister == WaspRegister) { return; }

  // Turn stuff off
  if (! (saved_state & UIO_MAXB))  { maxbotix(0); }
  if (! (saved_state & UIO_I2C))   { i2c(0); }
  if (! (saved_state & UIO_1WIRE)) { onewire(0); }
  if (! (saved_state & UIO_SDI12)) { sdi12(0); }
  if (batteryType == BATTERY_LEAD)
  {
    if (! (saved_state & UIO_3V3)) { v33(0); }
    if (! (saved_state & UIO_5V))  { v5(0); }
    if (! (saved_state & UIO_LEAD_VOLTAGE)) { leadVoltage(0); }
    if (! (saved_state & UIO_12V)) { v12(0); }
  }
  else
  {
    if (! (saved_WaspRegister & REG_3V3)) { v33(0); }
    if (! (saved_WaspRegister & REG_5V)) { v5(0); }
  }

  // Turn stuff on
  if (batteryType == BATTERY_LEAD)
  {
    if (saved_state & UIO_12V) { v12(1); }
    if (saved_state & UIO_3V3) { v33(1); }
    if (saved_state & UIO_5V)  { v5(1); }
    if (saved_state & UIO_LEAD_VOLTAGE) { leadVoltage(1); }
  }
  else
  {
    if (saved_WaspRegister & REG_3V3) { v33(1); }
    if (saved_WaspRegister & REG_5V)  { v5(1); }
  }
  if (saved_state & UIO_MAXB)  { maxbotix(1); }
  if (saved_state & UIO_I2C)   { i2c(1); }
  if (saved_state & UIO_1WIRE) { onewire(1); }
  if (saved_state & UIO_SDI12) { sdi12(1); }
}


/*
 * On/Off devices
 */

bool WaspUIO::_setState(uint8_t device, bool new_state)
{
  bool old_state = state & device;
  if (new_state) { state |= device; }
  else           { state &= ~device; }
  return old_state;
}

bool WaspUIO::_powerBoardSwitch(uint8_t device, uint8_t pin, bool new_state)
{
  bool old_state = state & device;
  if (new_state == old_state) { return old_state; }

  digitalWrite(pin, (new_state) ? HIGH: LOW);
  return _setState(device, new_state);
}

bool WaspUIO::v12(bool new_state)
{
  if (batteryType == BATTERY_LEAD)
  {
    return _powerBoardSwitch(UIO_12V, 16, new_state);
  }

  return false;
}

bool WaspUIO::v33(bool new_state)
{
  if (batteryType == BATTERY_LEAD)
  {
    if (new_state && ! v12(1)) {} // Switch V12 on if needed
    return _powerBoardSwitch(UIO_3V3, 14, new_state);
  }

  // Lithium
  bool old_state = WaspRegister & REG_3V3;
  if (new_state == old_state) { return old_state; }
  PWR.setSensorPower(SENS_3V3, new_state ? SENS_ON : SENS_OFF);
  return old_state;
}

bool WaspUIO::v5(bool new_state)
{
  if (batteryType == BATTERY_LEAD)
  {
    if (new_state && ! v12(1)) {} // Switch V12 on if needed
    return _powerBoardSwitch(UIO_5V, 15, new_state);
  }

  // Lithium
  bool old_state = WaspRegister & REG_5V;
  if (new_state == old_state) { return old_state; }
  PWR.setSensorPower(SENS_5V, new_state ? SENS_ON : SENS_OFF);
  return old_state;
}

bool WaspUIO::leadVoltage(bool new_state)
{
  if (batteryType == BATTERY_LEAD)
  {
    if (new_state && ! v12(1)) {} // Switch V12 on if needed
    return _powerBoardSwitch(UIO_LEAD_VOLTAGE, 17, new_state);
  }

  return false;
}

bool WaspUIO::maxbotix(bool new_state)
{
  uint8_t device = UIO_MAXB;
  uint8_t pin = DIGITAL1;

  bool old_state = state & device;
  if (new_state == old_state) { return old_state; }

  // Requires 5V or 3V3
  if (new_state)
  {
    if (boardType == BOARD_LEMMING) { v5(1); }
    if (boardType == BOARD_NONE)    { v33(1); }
  }

  // Switch
  if (boardType == BOARD_LEMMING)
  {
    _powerBoardSwitch(device, pin, new_state);
  }
  else
  {
    _setState(device, new_state);
  }

  return old_state;
}

bool WaspUIO::i2c(bool new_state)
{
  uint8_t device = UIO_I2C;
  uint8_t pin = DIGITAL2;

  bool old_state = state & device;
  if (new_state == old_state) { return old_state; }

  // Requires 3V3
  v33(1);

  if (boardType == BOARD_LEMMING)
  {
    _powerBoardSwitch(device, pin, new_state);
  }
  else
  {
    _setState(device, new_state);
  }

  return old_state;
}

bool WaspUIO::onewire(bool new_state)
{
  uint8_t device = UIO_1WIRE;
  uint8_t pin = DIGITAL5;

  bool old_state = state & device;
  if (new_state == old_state) { return old_state; }

  // Requires 5V or 3V3
  if (new_state)
  {
    if (boardType == BOARD_LEMMING) { v5(1); }
    if (boardType == BOARD_NONE)    { v33(1); }
  }

  // Switch
  if (boardType == BOARD_LEMMING)
  {
    _powerBoardSwitch(device, pin, new_state);
  }
  else
  {
    _setState(device, new_state);
  }

  return old_state;
}

bool WaspUIO::sdi12(bool new_state)
{
  uint8_t device = UIO_SDI12;
  uint8_t pin = DIGITAL7;

  bool old_state = state & device;
  if (new_state == old_state) { return old_state; }

  // Requires 5V
  if (new_state) { v5(1); }

  // Switch
  if (boardType == BOARD_LEMMING)
  {
    _powerBoardSwitch(device, pin, new_state);
  }
  else
  {
    _setState(device, new_state);
  }

/*
  if (new_state) { mySDI12.begin(); }
  else           { mySDI12.end(); }
*/

  return old_state;
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

  if (! leadVoltage(1)) { delay(100); } // Turn on, let power to stabilize
  analog5 = analogRead(ANALOG5);   // Analog output (0 - 3.3V): from 0 to 1023
  leadVoltage(0);                  // Turn off

  volts = analog5  * (R1 + R2) / R2 * 3.3 / 1023 ;
  Utils.float2String(volts, volts_str, 2);
  debug(F("Lead acid battery analog5=%d volts=%s"), analog5, volts_str);

  return volts;
}
