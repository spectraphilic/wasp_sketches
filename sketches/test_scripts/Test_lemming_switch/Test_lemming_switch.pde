/*
    ------ [PWR_5] - I/O Power Supply management --------

    Explanation: This example shows how to set up the power supply lines
    in the Waspmote I/O sockets

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
    Design:            David Gascon
    Implementation:    Yuri Carmona
*/


void setup()
{
  USB.ON();
  USB.println(F("PWR_5 example"));


  pinMode(DIGITAL1, OUTPUT); // Maxbotix power switch
  pinMode(DIGITAL2, OUTPUT); // I2C power switch
  pinMode(DIGITAL5, OUTPUT); // OneWire power switch
  pinMode(DIGITAL7, OUTPUT); // SDI-12 power switch


}

void loop()
{
//  USB.println(F("Switch on power supply lines"));
//  PWR.setSensorPower(SENS_5V, SENS_ON);
//  delay(3000);
//
//  USB.println(F("Digital control pin High"));
//  digitalWrite(DIGITAL1, HIGH);
//  delay(3000);
//
//  USB.println(F("Digital control pin Low"));
//  digitalWrite(DIGITAL1, LOW);
//  delay(3000);
//
//  USB.println(F("Switch off power supply lines"));
//  PWR.setSensorPower(SENS_5V, SENS_OFF);
//  delay(3000);

/////////////////////////////////////////////////////////////////////


    USB.println(F("Switch on power supply lines"));
  PWR.setSensorPower(SENS_3V3, SENS_ON);
  delay(3000);

  USB.println(F("Digital control pin High"));
  digitalWrite(DIGITAL2, HIGH);
  delay(3000);

  USB.println(F("Digital control pin Low"));
  digitalWrite(DIGITAL2, LOW);
  delay(3000);

  USB.println(F("Switch off power supply lines"));
  PWR.setSensorPower(SENS_3V3, SENS_OFF);
  delay(3000);
}

