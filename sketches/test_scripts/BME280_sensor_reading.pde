/*

   Modified by John Hulth. 2017-09-04

   meassure the BME-280 sensor directly on the waspmote

   BME-280____________Waspmote
   SCL     --->       SCL
   SDA     --->       SDA
   3.3v    --->       3.3 sensor power
   GND     --->       GND


     ------------  [Ga_v30_01] - Temperature, Humidty and Pressure  --------------

     Explanation: This example read the temperature, humidity and
     pressure values from BME280 sensor

     Copyright (C) 2016 Libelium Comunicaciones Distribuidas S.L.
     http://www.libelium.com

     This program is free software: you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published by
     the Free Software Foundation, either version 3 of the License, or
     (at your option) any later version.

     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details.

     You should have received a copy of the GNU General Public License
     along with this program.  If not, see <http://www.gnu.org/licenses/>.

     Version:           3.0
     Design:            David Gascón
     Implementation:    Ahmad Saad
*/

// Library include

#include "BME280.h"
#include <WaspFrame.h>

float temperature; // Stores the temperature in ºC
float humidity;	   // Stores the realitve humidity in %RH
float pressure;	   // Stores the pressure in Pa

char node_ID[] = "BME280_example";

void setup()
{
  USB.ON();
  USB.println(F("Temperature, Humidity and Pressure example"));
  // Set the Waspmote ID
  frame.setID(node_ID);

  ///////////////////////////////////////////
  // 1. Turn on the sensor
  ///////////////////////////////////////////

  // Switch ON and configure the BME sensor
  BME.ON();
  delay(100);
}

void loop()
{
  ///////////////////////////////////////////
  // 2. Read sensors
  ///////////////////////////////////////////

  // Read enviromental variables
  temperature = BME.getTemperature(BME280_OVERSAMP_1X, 0);
  humidity = BME.getHumidity(BME280_OVERSAMP_1X);
  pressure = BME.getPressure(BME280_OVERSAMP_1X, 0);

  // Print of the results
  USB.print(F("Temperature: "));
  USB.print(temperature);
  USB.print(F(" Celsius Degrees |"));

  USB.print(F(" Humidity : "));
  USB.print(humidity);
  USB.print(F(" %RH"));

  USB.print(F(" Pressure : "));
  USB.print(pressure);
  USB.print(F(" Pa"));

  USB.println();

  ///////////////////////////////////////////
  // 3. Create ASCII frame
  ///////////////////////////////////////////

  // Create new frame (ASCII)
  frame.createFrame(ASCII, node_ID);
  // Add temperature
  frame.addSensor(SENSOR_BME_TC, temperature);
  // Add humidity
  frame.addSensor(SENSOR_BME_HUM, humidity);
  // Add pressure
  frame.addSensor(SENSOR_BME_PRES, pressure);
  // Show the frame
  frame.showFrame();

  delay(3000);
}

