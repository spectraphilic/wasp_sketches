#include "WaspUIO.h"

/*
 * Menu: wake-up frequency
 */

void WaspUIO::menuActions()
{
  char str[80];

  do
  {
    cr.print();
    cr.print(F("Time in minutes, from 0 (inactive) to 255"));
    cr.print(F("0. Network: %d"), action_network);
#if USE_AGR
    cr.print(F("1. Agr board: Sensirion (%s)"), action_sensirion);
    cr.print(F("2. Agr board: Pressure (%s)"), action_pressure);
    cr.print(F("3. Agr board: Leaf wetness (%s)"), action_leafwetness);
#endif
#if USE_I2C
    cr.print(F("4. I2C: BME-280 (%s)"), action_bme280);
    cr.print(F("5. OneWire: DS1820 (%s)"), action_ds1820);
    cr.print(F("6. TTL: MB7389 (%s)"), action_mb);
#endif
#if USE_SDI
    cr.print(F("7. SDI-12: CTD-10 (%s)"), action_ctd10);
    cr.print(F("8. SDI-12: DS-2 (%s)"), action_ds2);
#endif
    cr.print(F("9. Exit"));
    cr.print();
    input(str, sizeof(str), F("==> Enter numeric option:"), 0);
    if (strlen(str) == 0)
      continue;

    switch (str[0])
    {
      case '0': menuAction(EEPROM_ACTION_NETWORK, action_network); break;
      case '1': menuAction(EEPROM_ACTION_NETWORK, action_sensirion); break;
      case '2': menuAction(EEPROM_ACTION_NETWORK, action_pressure); break;
      case '3': menuAction(EEPROM_ACTION_NETWORK, action_leafwetness); break;
      case '4': menuAction(EEPROM_ACTION_NETWORK, action_bme280); break;
      case '5': menuAction(EEPROM_ACTION_NETWORK, action_ds1820); break;
      case '6': menuAction(EEPROM_ACTION_NETWORK, action_mb); break;
      case '7': menuAction(EEPROM_ACTION_NETWORK, action_ctd10); break;
      case '8': menuAction(EEPROM_ACTION_NETWORK, action_ds2); break;
      case '9': cr.print(); return;
    }
  } while (true);
}


void WaspUIO::menuAction(uint16_t address, uint8_t &wakeup)
{
  char str[80];
  int value;

  do
  {
    input(str, sizeof(str), F("Enter the wake period in minutes (0-255). Press Enter to leave it unchanged:"), 0);
    if (strlen(str) == 0)
      return;

    // Set new time
    if (sscanf(str, "%d", &value) == 1)
    {
      if (value >= 0 && value <= 255)
      {
        wakeup = (uint8_t) value;
        updateEEPROM(address, wakeup);
        return;
      }
    }
  } while (true);
}
