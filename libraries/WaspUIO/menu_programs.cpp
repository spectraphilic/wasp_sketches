#include "WaspUIO.h"

/*
 * Menu: wake-up frequency
 */

void WaspUIO::menuPrograms()
{
  char str[80];

  do
  {
    cr.print();
    cr.print(F("1. Sensors: %d minutes"), wakeup_sensors);
    cr.print(F("2. Network: %d minutes"), wakeup_network);
    cr.print(F("9. Exit"));
    cr.print();
    input(str, sizeof(str), F("==> Enter numeric option:"), 0);
    if (strlen(str) == 0)
      continue;

    switch (str[0])
    {
      case '1':
        menuProgramWakeup(EEPROM_WAKEUP_SENSORS, wakeup_sensors); break;
      case '2':
        menuProgramWakeup(EEPROM_WAKEUP_NETWORK, wakeup_network); break;
      case '9':
        cr.print();
        return;
    }
  } while (true);
}


void WaspUIO::menuProgramWakeup(uint16_t address, uint8_t &wakeup)
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
        fixWakeup();
        return;
      }
    }
  } while (true);
}
