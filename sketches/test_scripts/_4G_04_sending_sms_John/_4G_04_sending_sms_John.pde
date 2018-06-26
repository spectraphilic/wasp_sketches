/*
    
Added 'Enter pin code', then it worked!!!!
    
    --------------- 4G_04 - Sending SMS   ---------------

    Explanation: This example shows how to set up the module to use
    SMS and send text messages

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
    Implementation:    Alejandro Gállego
*/

#include <Wasp4G.h>

//////////////////////////////////////////////////
char phone_number[] = "41392555"; // John H. phone# 004741392555
char sms_body[] = "Hello from Waspmote!!!";
//////////////////////////////////////////////////

uint8_t error;


void setup()
{
  USB.ON();
  USB.println(F("Start program"));
}


void loop()
{
  //////////////////////////////////////////////////
  // 1. Switch on the 4G module
  //////////////////////////////////////////////////
  error = _4G.ON();

  if (error == 0)
  {
    USB.println(F("1. 4G module ready..."));

    ////////////////////////////////////////////////
    // Enter PIN code
    ////////////////////////////////////////////////
    USB.println(F("Setting PIN code..."));
    if (_4G.enterPIN("1031") == 0)
    {
      USB.println(F("PIN code accepted"));
    }
    else
    {
      USB.println(F("PIN code incorrect"));
    }

    delay(1000);

    ////////////////////////////////////////////////
    // 2. Configure SMS options
    ////////////////////////////////////////////////
    error = _4G.configureSMS();

    if (error == 0)
    {
      USB.println(F("2.1. 4G module configured to use SMS"));
    }
    else
    {
      USB.print(F("2.1. Error calling 'configureSMS' function. Code: "));
      USB.println(error, DEC);
    }


    ////////////////////////////////////////////////
    // 3. Send SMS
    ////////////////////////////////////////////////
    USB.print(F("2.2. Sending SMS..."));
    error = _4G.sendSMS( phone_number, sms_body);

    if (error == 0)
    {
      USB.println(F("done"));
    }
    else
    {
      USB.print(F("error. Code: "));
      USB.println(error, DEC);
    }
  }
  else
  {
    // Problem with the communication with the 4G module
    USB.print(F("1. 4G module not started. Error code: "));
    USB.println(error, DEC);
  }


  ////////////////////////////////////////////////
  // 3. Powers off the 4G module
  ////////////////////////////////////////////////
  USB.println(F("3. Switch OFF 4G module"));
  _4G.OFF();


  ////////////////////////////////////////////////
  // 4. Sleep
  ////////////////////////////////////////////////
  USB.println(F("4. Enter deep sleep..."));
  PWR.deepSleep("00:01:00:00", RTC_OFFSET, RTC_ALM1_MODE1, ALL_OFF);

  USB.ON();
  USB.println(F("5. Wake up!!\n\n"));

}





