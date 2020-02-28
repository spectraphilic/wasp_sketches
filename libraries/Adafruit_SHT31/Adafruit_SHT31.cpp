/*!
 *  @file Adafruit_SHT31.cpp
 *
 *  @mainpage Adafruit SHT31 Digital Humidity & Temp Sensor
 *
 *  @section intro_sec Introduction
 *
 *  This is a library for the SHT31 Digital Humidity & Temp Sensor
 *
 *  Designed specifically to work with the SHT31 Digital sensor from Adafruit
 *
 *  Pick one up today in the adafruit shop!
 *  ------> https://www.adafruit.com/product/2857
 *
 *  These sensors use I2C to communicate, 2 pins are required to interface
 *
 *  Adafruit invests time and resources providing this open source code,
 *  please support Adafruit andopen-source hardware by purchasing products
 *  from Adafruit!
 *
 *  @section author Author
 *
 *  Limor Fried/Ladyada (Adafruit Industries).
 *
 *  @section license License
 *
 *  BSD license, all text above must be included in any redistribution
 */

#include "Adafruit_SHT31.h"

/*!
 * @brief  SHT31 constructor using i2c
 * @param  *theWire
 *         optional wire
 */
Adafruit_SHT31::Adafruit_SHT31() {
  _i2caddr = 0;

  humidity = NAN;
  temp = NAN;
}

bool Adafruit_SHT31::begin(uint8_t i2caddr) {
  I2C.begin();
  _i2caddr = i2caddr;
  reset();
  return readStatus() != 0xFFFF;
}

uint16_t Adafruit_SHT31::readStatus(void) {
  uint8_t data[3];
  if (I2C.read(_i2caddr, SHT31_READSTATUS, data, 3))
    return 0xFFFF; // Error

  uint16_t stat = ((uint16_t)data[0] << 8 | data[1]);
  // Serial.println(stat, HEX);
  return stat;
}

void Adafruit_SHT31::reset(void) {
  writeCommand(SHT31_SOFTRESET);
  delay(10);
}

void Adafruit_SHT31::heater(bool h) {
  if (h)
    writeCommand(SHT31_HEATEREN);
  else
    writeCommand(SHT31_HEATERDIS);
}

float Adafruit_SHT31::readTemperature(void) {
  if (!readTempHum())
    return NAN;

  return temp;
}

float Adafruit_SHT31::readHumidity(void) {
  if (!readTempHum())
    return NAN;

  return humidity;
}

/**
 * Performs a CRC8 calculation on the supplied values.
 *
 * @param data  Pointer to the data to use when calculating the CRC8.
 * @param len   The number of bytes in 'data'.
 *
 * @return The computed CRC8 value.
 */
static uint8_t crc8(const uint8_t *data, int len) {
  /*
   *
   * CRC-8 formula from page 14 of SHT spec pdf
   *
   * Test data 0xBE, 0xEF should yield 0x92
   *
   * Initialization data 0xFF
   * Polynomial 0x31 (x8 + x5 +x4 +1)
   * Final XOR 0x00
   */

  const uint8_t POLYNOMIAL(0x31);
  uint8_t crc(0xFF);

  for (int j = len; j; --j) {
    crc ^= *data++;

    for (int i = 8; i; --i) {
      crc = (crc & 0x80) ? (crc << 1) ^ POLYNOMIAL : (crc << 1);
    }
  }
  return crc;
}

bool Adafruit_SHT31::readTempHum(void) {
  uint8_t readbuffer[6];

  writeCommand(SHT31_MEAS_HIGHREP);

  delay(20);
  if (I2C.read(_i2caddr, readbuffer, sizeof(readbuffer)))
    return false;

  if (readbuffer[2] != crc8(readbuffer, 2) ||
      readbuffer[5] != crc8(readbuffer + 3, 2))
    return false;

  int32_t stemp = (int32_t) (((uint32_t)readbuffer[0] << 8) | readbuffer[1]);
  // simplified (65536 instead of 65535) integer version of:
  //temp = (stemp * 175.0f) / 65535.0f - 45.0f;
  stemp = ((4375 * stemp) >> 14) - 4500;
  temp = (float) stemp / 100.0f;

  uint32_t shum = ((uint32_t)readbuffer[3] << 8) | readbuffer[4];
  // simplified (65536 instead of 65535) integer version of:
  //humidity = (shum * 100.0f) / 65535.0f;
  shum = (625 * shum) >> 12;
  humidity = (float) shum / 100.0f;

  return true;
}

void Adafruit_SHT31::writeCommand(uint16_t cmd) {
  uint8_t reversed[2] = {(uint8_t)(cmd >> 8), (uint8_t)(cmd & 0xFF)};
  I2C.write(_i2caddr, reversed, 2);
}
