#include "WaspUIO.h"


/**
 * Scan I2C slaves
 */

COMMAND(cmdI2C)
{
  // Parse command line
  char name[20];
  int n = sscanf(str, "%19s", name);
  if (n != -1 && n != 1) { return cmd_bad_input; }

  // Power ON
  UIO.i2c(1);

  // Scan
  if (n == -1)
  {
    UIO.i2c_scan();
    return cmd_quiet;
  }

  // Read
  bool error;
  int8_t value = UIO.index(run_names, sizeof run_names / sizeof run_names[0], name);
  if (value == 7)
  {
    float temperature, humidity, pressure;
    error = UIO.i2c_BME280(temperature, humidity, pressure);
  }
  else if (value == 10)
  {
    uint8_t temp;
    float r, s, t, u, v, w;
    error = UIO.i2c_AS7263(temp, r, s, t, u, v, w);
  }
  else if (value == 11)
  {
    uint8_t temp;
    float A, B, C, D, E, F, G, H, I, J, K, L, R, S, T, U, V, W;
    error = UIO.i2c_AS7265(temp, A, B, C, D, E, F, G, H, I, J, K, L, R, S, T, U, V, W);
  }
  else if (value == 12)
  {
    float temperature, humidity, pressure;
    error = UIO.i2c_BME280(temperature, humidity, pressure, I2C_ADDRESS_LAGOPUS_BME280);
  }
  else if (value == 13)
  {
    float object, ambient;
    error = UIO.i2c_MLX90614(object, ambient);
  }
  else if (value == 14)
  {
    float temperature;
    error = UIO.i2c_TMP102(temperature);
  }
  else if (value == 15)
  {
    uint16_t distance;
    error = UIO.i2c_VL53L1X(distance);
  }
  else
  {
    return cmd_bad_input;
  }

  return (error)? cmd_error: cmd_quiet;
}



/**
 * Read sensor now
 */

COMMAND(cmdMB)
{
  uint16_t median, sd;
  bool error = UIO.readMaxbotixSerial(median, sd, 5);
  if (error)
  {
    return cmd_error;
  }

  return cmd_ok;
}


/**
 * OneWire DS18B20 string: read
 */

COMMAND(cmd1WireRead)
{
  uint8_t max = 99;
  int values[max];
  UIO.readDS18B20(values, max);

  return cmd_ok;
}


/**
 * OneWire DS18B20 string: scan the given pins
 * Save to onewire.txt
 */

uint8_t _getPin(uint8_t pin)
{
  // 0 - 8
  if (pin == 0) return DIGITAL0;
  if (pin == 1) return DIGITAL1;
  if (pin == 2) return DIGITAL2;
  if (pin == 3) return DIGITAL3;
  if (pin == 4) return DIGITAL4;
  if (pin == 5) return DIGITAL5;
  if (pin == 6) return DIGITAL6;
  if (pin == 7) return DIGITAL7;
  if (pin == 8) return DIGITAL8;

  return 255;
}

COMMAND(cmd1WireScan)
{
  uint8_t npins;
  uint8_t pins[] = {255, 255, 255};
  uint8_t pin;
  uint8_t addr[8];
  char addr_str[17];
  uint8_t crc;
  SdFile file;
  bool has_file = false;
  size_t size = 20;
  char buffer[size];

  // Check input
  npins = sscanf(str, "%hhu %hhu %hhu", &pins[0], &pins[1], &pins[2]);
  if (npins < 1) { return cmd_bad_input; }

  // ON
  if (UIO.hasSD)
  {
    if (SD.openFile("onewire.txt", &file, O_WRITE | O_CREAT | O_TRUNC))
    {
      has_file = true;
    }
    else
    {
      cr.print(F("Error opening onewire.txt"));
    }
  }

  if (! UIO.onewire(1)) { delay(750); }

  for (uint8_t i = 0; i < npins; i++)
  {
    pin = _getPin(pins[i]);
    if (pin == 255) continue;

    snprintf_F(buffer, size, F("%hhu"), pins[i]);
    USB.print(buffer); if (has_file) file.write(buffer);
    WaspOneWire oneWire(pin);

    // For now we only support the DS1820, so here just read that directly
    // We assume we have a chain of DS1820 sensors, and read all of them.
    if (! oneWire.reset())
    {
      cr.print(F(" nothing"));
      goto next;
    }

    // Search
    oneWire.reset_search();
    while (oneWire.search(addr))
    {
      Utils.hex2str(addr, addr_str, 8);
      snprintf_F(buffer, size, F(" %s"), addr_str);
      USB.print(buffer); if (has_file) file.write(buffer);

      // Check address CRC
      crc = oneWire.crc8(addr, 7);
      if (crc != addr[7]) { cr.print(F("(crc error)")); continue; }

      // Only DS18B20 is supported for now
      if (addr[0] != 0x28) { cr.print(F("(not DS18B20)")); continue; }
    }

next:
    oneWire.depower();
    USB.println(); if (has_file) file.write("\n");
  }

  // OFF
  UIO.onewire(0);
  if (has_file) { file.close(); }

  return cmd_quiet;
}


/**
 * SDI-12
 */

COMMAND(cmdSDI12)
{
  uint8_t address, new_address;

  int n = sscanf(str, "%hhu %hhu", &address, &new_address);

  UIO.sdi12(1);
  if (n <= 0)
  {
    UIO.sdi_read_address();
  }
  else if (n == 1)
  {
    UIO.sdi_identify(address);
  }
  else if (n == 2)
  {
    UIO.sdi_set_address(address, new_address);
  }
  UIO.sdi12(0);

  return cmd_quiet;
}
