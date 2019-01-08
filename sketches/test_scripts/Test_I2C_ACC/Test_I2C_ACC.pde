/*
    ------ [ACC_1] Waspmote Accelerometer Reading acceleration --------

    Explanation: This example shows how to get the acceleration on the
    different axis using the most basic functions related with Waspmote
    accelerometer

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
    Implementation:    Marcos Yarza
*/


uint8_t status;
int x_acc;
int y_acc;
int z_acc;

#include "SparkFun_VL53L1X_Arduino_Library.h"
VL53L1X distanceSensor;




void setup()
{
  ACC.ON();
  USB.ON(); // starts using the serial port
  RTC.ON();
  USB.println(F("ACC_01 example"));
  PWR.setSensorPower(SENS_3V3, SENS_ON);
  pinMode(DIGITAL2, OUTPUT); // I2C power switch
  digitalWrite(DIGITAL2, HIGH);
  delay(1000);

  if (distanceSensor.begin() == false)
    USB.print(F("Distance Sensor offline!"));


}

void loop()
{

  //----------Check Register-----------------------
  // should always answer 0x32, it is used to check
  // the proper functionality of the accelerometer
  status = ACC.check();

  //----------X Value-----------------------
  x_acc = ACC.getX();

  //----------Y Value-----------------------
  y_acc = ACC.getY();

  //----------Z Value-----------------------
  z_acc = ACC.getZ();

  //-------------------------------

  USB.print(F("\n------------------------------\nCheck: 0x"));
  USB.println(status, HEX);
  USB.println(F("\n \t0X\t0Y\t0Z"));
  USB.print(F(" ACC\t"));
  USB.print(x_acc, DEC);
  USB.print(F("\t"));
  USB.print(y_acc, DEC);
  USB.print(F("\t"));
  USB.println(z_acc, DEC);

  delay(1000);

  //----------------------------------------------------------------

  //Poll for completion of measurement. Takes 40-50ms.
  while (distanceSensor.newDataReady() == false)
    delay(5);

  int distance = distanceSensor.getDistance(); //Get the result of the measurement from the sensor

  USB.print("Distance(mm): ");
  USB.print(distance);

  USB.println();

  //----------------------------------------------------------------

}
