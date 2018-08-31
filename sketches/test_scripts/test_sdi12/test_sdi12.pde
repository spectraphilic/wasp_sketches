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
#include "SDI12.h"

WaspSDI12 sdi12(DIGITAL8);

void setup()
{
  // put your setup code here, to run once:

}


void loop()
{
  char buffer[65];
  int n;
  int c;
  int i;

  USB.println("Loop...");

  PWR.setSensorPower(SENS_5V, SENS_ON);
  delay(500);

  sdi12.sendCommand("?!");
  sdi12.identify(0);

  USB.println("");
  PWR.deepSleep("00:00:00:10", RTC_OFFSET, RTC_ALM1_MODE1, ALL_OFF); // 10s
}
