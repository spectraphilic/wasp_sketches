#include "WaspUIO.h"


/*
 * On/Off devices
 */

bool WaspUIO::on(uint8_t device)
{
  // If already On, do nothing
  if (state & device) { return false; }

  // Mark as On
  state |= device;

  // Turn on
  switch (device)
  {
    case UIO_V12: // 12V_SW ON/OFF SWITCH
      pinMode(ANA2, OUTPUT);
      digitalWrite(ANA2, HIGH);
      break;
    case UIO_V5:
      break;
    case UIO_V33:
      break;
    case UIO_LEAD_VOLTAGE: // Lead-Acid meassurement ON/OFF SWITCH
      pinMode(ANA3, OUTPUT);
      digitalWrite(ANA3, HIGH);
      break;
    case UIO_MAXB:
      break;
    case UIO_I2C:
      break;
    case UIO_1WIRE:
      break;
    case UIO_SDI12:
      mySDI12.begin();
      break;
  }

  return true;
}

bool WaspUIO::off(uint8_t device)
{
  // If already On, do nothing
  if (! (state & device)) { return false; }

  state &= ~device;

  switch (device)
  {
    case UIO_V12: // 12V_SW ON/OFF SWITCH
      pinMode(ANA2, OUTPUT);
      digitalWrite(ANA2, LOW);
      break;
    case UIO_V5:
      break;
    case UIO_V33:
      break;
    case UIO_LEAD_VOLTAGE: // Lead-Acid meassurement ON/OFF SWITCH
      pinMode(ANA3, OUTPUT);
      digitalWrite(ANA3, LOW);
      break;
    case UIO_MAXB:
      break;
    case UIO_I2C:
      break;
    case UIO_1WIRE:
      break;
    case UIO_SDI12:
      mySDI12.end();
      break;
  }

  return true;
}

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
  bool change_v12 = on(UIO_V12);
  bool change_la = on(UIO_LEAD_VOLTAGE);

  // Analog output (0 - 3.3V): from 0 to 1023
  pinMode(ANALOG5, INPUT);  //ANALOG5 Read data
  delay(100); // Let power to stabilize
  analog5 = analogRead(ANALOG5);

  // Turn off
  if (change_la) { off(UIO_LEAD_VOLTAGE); }
  if (change_v12) { off(UIO_V12); }

  volts = analog5  * (R1 + R2) / R2 * 3.3 / 1023 ;
  Utils.float2String(volts, volts_str, 2);
  debug(F("Lead acid battery analog5=%d volts=%s"), analog5, volts_str);

  return volts;
}
