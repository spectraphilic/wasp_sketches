#include "WaspUIO.h"


void WaspUIO::startPowerBoard()
{
  pinMode(14, OUTPUT); // 3V3 switch
  pinMode(15, OUTPUT); // 5V switch
  pinMode(16, OUTPUT); // 12V switch
  pinMode(17, OUTPUT); // Lead-Acid meassurement switch
  pinMode(ANALOG5, INPUT);  // Lead-Acid measurement input line
}

void WaspUIO::startSensorBoard()
{
  pinMode(ANA0, OUTPUT); // I2C isolator
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
 * On/Off devices
 */
bool WaspUIO::_powerBoardSwitch(uint8_t device, uint8_t pin, bool new_state)
{
  // Safeguard, don't call this if not using the sensor board
  if (boardType != BOARD_LEMMING)
  {
    return false;
  }

  bool old_state = state & device;
  if (new_state == old_state) { return old_state; }

  if (new_state)
  {
    state |= device;
    digitalWrite(pin, HIGH);
  }
  else
  {
    state &= ~device;
    digitalWrite(pin, LOW);
  }

  return old_state;
}

bool WaspUIO::v12(bool new_state)
{
  return _powerBoardSwitch(UIO_12V, 16, new_state);
}

bool WaspUIO::v33(bool new_state)
{
  if (boardType == BOARD_LEMMING)
  {
    if (new_state && ! v12(1)) {} // Switch V12 on if needed
    return _powerBoardSwitch(UIO_3V3, 14, new_state);
  }

  // Without board
  bool old_state = WaspRegister & REG_3V3;
  if (new_state == old_state) { return old_state; }
  PWR.setSensorPower(SENS_3V3, new_state ? SENS_ON : SENS_OFF);
  return old_state;
}

bool WaspUIO::v5(bool new_state)
{
  if (boardType == BOARD_LEMMING)
  {
    if (new_state && ! v12(1)) {} // Switch V12 on if needed
    return _powerBoardSwitch(UIO_3V3, 14, new_state);
    return _powerBoardSwitch(UIO_5V, 15, new_state);
  }

  // Without board
  bool old_state = WaspRegister & REG_3V3;
  if (new_state == old_state) { return old_state; }
  PWR.setSensorPower(SENS_5V, new_state ? SENS_ON : SENS_OFF);
  return old_state;
}

bool WaspUIO::leadVoltage(bool new_state)
{
  if (new_state && ! v12(1)) {} // Switch V12 on if needed
  return _powerBoardSwitch(UIO_LEAD_VOLTAGE, 17, new_state);
}

bool WaspUIO::maxbotix(bool new_state)
{
  uint8_t device = UIO_MAXB;
  bool old_state = state & device;
  if (new_state == old_state) { return old_state; }

  if (new_state)
  {
    state |= device;
    if (boardType == BOARD_LEMMING)
    {
      if (! v5(1)) { delay(1000); }
      digitalWrite(DIGITAL1, HIGH);
    }
    else
    {
      if (! v33(1)) { delay(1000); }
    }
  }
  else
  {
    state &= ~device;
    if (boardType == BOARD_LEMMING)
    {
      digitalWrite(DIGITAL1, LOW);
    }
  }

  return old_state;
}

bool WaspUIO::i2c(bool new_state)
{
  uint8_t device = UIO_I2C;
  bool old_state = state & device;
  if (new_state == old_state) { return old_state; }

  // XXX Do we have an I2C isolator
  if (new_state)
  {
    state |= device;
    if (boardType == BOARD_LEMMING)
    {
      if (! v5(1)) { delay(1000); }
      digitalWrite(DIGITAL2, HIGH);
    }
    else
    {
      if (! v33(1)) { delay(1000); }
    }
  }
  else
  {
    state &= ~device;
    if (boardType == BOARD_LEMMING)
    {
      digitalWrite(DIGITAL2, LOW);
    }
  }

  return old_state;
}

bool WaspUIO::onewire(bool new_state)
{
  uint8_t device = UIO_1WIRE;
  bool old_state = state & device;
  if (new_state == old_state) { return old_state; }

  if (new_state)
  {
    state |= device;
    if (boardType == BOARD_LEMMING)
    {
      if (! v5(1)) { delay(1000); }
      digitalWrite(DIGITAL5, HIGH);
    }
    else
    {
      if (! v33(1)) { delay(1000); }
    }
  }
  else
  {
    state &= ~device;
    if (boardType == BOARD_LEMMING)
    {
      digitalWrite(DIGITAL5, LOW);
    }
  }

  return old_state;
}

bool WaspUIO::sdi12(bool new_state)
{
  uint8_t device = UIO_SDI12;
  bool old_state = state & device;
  if (new_state == old_state) { return old_state; }

  if (new_state)
  {
    state |= device;
    if (! v5(1)) { delay(1000); }
    if (boardType == BOARD_LEMMING)
    {
      digitalWrite(DIGITAL7, HIGH);
    }
    mySDI12.begin();
  }
  else
  {
    state &= ~device;
    if (boardType == BOARD_LEMMING)
    {
      digitalWrite(DIGITAL7, LOW);
    }
    mySDI12.end();
  }

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

  // Turn on
  bool old_state_v12 = v12(1);
  bool old_state_lv = leadVoltage(1);
  if (!old_state_v12 || !old_state_lv) { delay(100); } // Let power to stabilize

  // Analog output (0 - 3.3V): from 0 to 1023
  analog5 = analogRead(ANALOG5);

  // Turn off
  leadVoltage(old_state_lv);
  v12(old_state_v12);

  volts = analog5  * (R1 + R2) / R2 * 3.3 / 1023 ;
  Utils.float2String(volts, volts_str, 2);
  debug(F("Lead acid battery analog5=%d volts=%s"), analog5, volts_str);

  return volts;
}
