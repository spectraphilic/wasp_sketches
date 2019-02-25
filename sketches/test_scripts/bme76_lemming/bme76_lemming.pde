/*
    ------ Waspmote Pro Code Example --------

    Explanation: This is the basic Code for Waspmote Pro

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
*/

// Put your libraries here (#include ...)
#include <BME280.h>


BME280 bme(0x76);
float temperature;
float humidity;
float pressure;

char filename[13];
uint8_t sd_answer;
char sample_txt[100];
uint16_t size;




void setup()
{
  
  // put your setup code here, to run once:
  digitalWrite(DIGITAL2, HIGH);
  delay(100);
  bme.ON();
  RTC.ON();
  SD.ON();
  delay(100);
  USB.println("Sensors ON");

  sprintf(filename, "%02u%02u%02u%02u.csv", RTC.date, RTC.hour, RTC.minute, RTC.second);
  USB.println(filename);
  sd_answer = SD.create(filename);
  USB.print("File written to SD O/I: ");
  USB.println(sd_answer);
  delay(100);
  

  if (bme.checkID() != 1)
  {
    USB.println("BME checked");
  }

  // Read the calibration registers
  // XXX Can this be done once in the setup?
  bme.readCalibration();

}


void loop()
{
  RTC.getTime();
  // Read
  float temperature = bme.getTemperature(BME280_OVERSAMP_1X, 0);
  float humidity = bme.getHumidity(BME280_OVERSAMP_1X);
  float pressure = bme.getPressure(BME280_OVERSAMP_1X, 0);

  USB.print("Temperature = ");
  USB.println(temperature);
  USB.print("Humidity = ");
  USB.println(humidity);
  USB.print("Pressure = ");
  USB.println(pressure);

  snprintf(sample_txt, sizeof(sample_txt), "\n20%02u-%02u-%02u %02u:%02u:%02u", RTC.year, RTC.month, RTC.date, RTC.hour, RTC.minute, RTC.second );
  SD.append(filename, sample_txt);
  USB.println(sample_txt);

   SD.append(filename, ", ");
  dtostrf(temperature, 2, 4, sample_txt);
  SD.append(filename, sample_txt);
  USB.println(sample_txt);

  SD.append(filename, ", ");
  dtostrf(humidity, 2, 4, sample_txt);
  SD.append(filename, sample_txt);
  USB.println(sample_txt);

  SD.append(filename, ", ");
  dtostrf(pressure, 2, 4, sample_txt);
  SD.append(filename, sample_txt);
  USB.println(sample_txt);

  delay(200);

}
