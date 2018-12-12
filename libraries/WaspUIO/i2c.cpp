#include "WaspUIO.h"


/** readBME280
 *
 * Returns: bool      - 0 if success, 1 if error
 */

bool WaspUIO::readBME280(float &temperature, float &humidity, float &pressure)
{
  if (BME.checkID() != 1)
  {
    return 1;
  }

  // Read the calibration registers
  // XXX Can this be done once in the setup?
  BME.readCalibration();

  // Read
  temperature = BME.getTemperature(BME280_OVERSAMP_1X, 0);
  humidity = BME.getHumidity(BME280_OVERSAMP_1X);
  pressure = BME.getPressure(BME280_OVERSAMP_1X, 0);

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
