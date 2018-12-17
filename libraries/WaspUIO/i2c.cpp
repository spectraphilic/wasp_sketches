#include "WaspUIO.h"
#include "SparkFun_VL53L1X_Arduino_Library.h"


/** readBME280
 *
 * Returns: bool      - 0 if success, 1 if error
 */

bool WaspUIO::readBME280(float &temperature, float &humidity, float &pressure, uint8_t address)
{
  BME280 bme(address);

  if (bme.checkID() != 1)
  {
    return 1;
  }

  // Read the calibration registers
  // XXX Can this be done once in the setup?
  bme.readCalibration();

  // Read
  temperature = bme.getTemperature(BME280_OVERSAMP_1X, 0);
  humidity = bme.getHumidity(BME280_OVERSAMP_1X);
  pressure = bme.getPressure(BME280_OVERSAMP_1X, 0);

  // Debug
  char aux[20];
  Utils.float2String(temperature, aux, 2);
  debug(F("BME-280 Temperature: %s Celsius Degrees"), aux);
  Utils.float2String(humidity, aux, 2);
  debug(F("BME-280 Humidity   : %s %%RH"), aux);
  Utils.float2String(pressure, aux, 2);
  debug(F("BME-280 Pressure   : %s Pa"), aux);

  return 0;
}


/** readVL53L1X
 *
 * Returns: bool      - 0 if success, 1 if error
 *
 * This code is derived from
 * https://github.com/sparkfun/SparkFun_VL53L1X_Arduino_Library/tree/master/examples/Example1_ReadDistance
 */

bool WaspUIO::readVL53L1X(uint16_t &distance)
{
  VL53L1X distanceSensor;

  if (distanceSensor.begin() == false)
  {
    return 1;
  }

  // Poll for completion of measurement. Takes 40-50ms.
  while (distanceSensor.newDataReady() == false)
    delay(5);

  distance = distanceSensor.getDistance();
  debug(F("Distance(mm): %u"), distance);

  return 0;
}
