/*  
 *  ------ [OTA_01] - OTA basic program with XBee DigiMesh -------- 
 *  
 *  Explanation: This program shows how build an program which supports
 *  Over The Air Programming (OTA) using XBee modules
 *  
 *  Copyright (C) 2012 Libelium Comunicaciones Distribuidas S.L. 
 *  http://www.libelium.com 
 *  
 *  This program is free software: you can redistribute it and/or modify 
 *  it under the terms of the GNU General Public License as published by 
 *  the Free Software Foundation, either version 3 of the License, or 
 *  (at your option) any later version. 
 *  
 *  This program is distributed in the hope that it will be useful, 
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of 
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 *  GNU General Public License for more details. 
 *  
 *  You should have received a copy of the GNU General Public License 
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>. 
 *  
 *  Version:           0.1 
 *  Design:            David Gascón 
 *  Implementation:    Yuri Carmona
 */
 

#include <WaspXBeeDM.h>
#include <WaspFrame.h>
char RX_ADDRESS[] = "0013a20040db6048";

#define key_access "LIBELIUM"
#define id_mote "WASPMOTE00000001"

void setup()
{
  USB.ON();
  USB.println("OTA_01_DM example");

  // Write Authentication Key to EEPROM memory
  Utils.setAuthKey(key_access);
  
  // Write Mote ID to EEPROM memory
  Utils.setID(id_mote);

  // Initialize XBee module
  xbeeDM.ON();
   
  // CheckNewProgram is mandatory in every OTA program
  xbeeDM.checkNewProgram();  
}

void loop()
{
  // Check if new data is available
  if( xbeeDM.available() )
  {
    xbeeDM.treatData();
    // Keep inside this loop while a new program is being received
    while( xbeeDM.programming_ON  && !xbeeDM.checkOtapTimeout() )
    {
      if( xbeeDM.available() )
      {
        xbeeDM.treatData();
      }
    }
  }
  
  // Blink LED1 while messages are not received
  USB.println("I am here !!");
  delay(100);
  USB.println("-----------");
  delay(100);
  
  frame.createFrame(ASCII);
  frame.addSensor(SENSOR_TIME, RTC.getTime() ); 
  frame.addSensor(SENSOR_STR,"drop");
  frame.showFrame();
  xbeeDM.send( RX_ADDRESS, frame.buffer, frame.length );
  delay(100);
  
    
}


