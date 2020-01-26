
// Put your libraries here (#include ...)
#include <BME280.h>

#define address     0x76

void setup()
{
  // put your setup code here, to run once:

  BME280 bme(address);

  // Read the calibration registers
  // XXX Can this be done once in the setup?
  bme.readCalibration();

  // Read
  float temperature = bme.getTemperature(BME280_OVERSAMP_1X, 0);
  float humidity = bme.getHumidity(BME280_OVERSAMP_1X);
  float pressure = bme.getPressure(BME280_OVERSAMP_1X, 0);
  USB.print("Temperature: ");
  USB.println(temperature);
  USB.print("Humidity: ");
  USB.println(humidity);
  USB.print("Pressure: ");
  USB.println(pressure);
}



void loop()
{
  // put your main code here, to run repeatedly:

}

