/*  
 *  ------ [DM_04b] - receive broadcast packets  -------- 
 *  
 *  Explanation: This program shows how to receive packets with 
 *  XBee-Digimesh modules. The packets received were broadcast
 *  
 *  Copyright (C) 2015 Libelium Comunicaciones Distribuidas S.L. 
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
 *  Version:           0.2
 *  Design:            David Gascón 
 *  Implementation:    Yuri Carmona
 */

#include <WaspXBeeDM.h>

// define variable
uint8_t error;
int savePower = 0;



void setup()
{  
  // init USB port
  USB.ON();
  USB.println(F("Receiving BROADCAST packets"));

  USB.print(PWR.getBatteryLevel(),DEC);
  USB.println(F(" %"));

  // init XBee 
  
}



void loop()
{ 
  if(savePower == 1){
  RTC.getTime();
  if((RTC.minute == 59)||(RTC.minute == 19)||(RTC.minute == 39)){
    long starttime = millis();
    long interval = 120000;
    long currenttime = millis();

    xbeeDM.ON();
    while(currenttime - starttime <= interval){
    
        
       // receive XBee packet (wait for 10 seconds)
      error = xbeeDM.receivePacketTimeout( 10000 );
    
      // check answer  
      if( error == 0 ) 
      {
        // print frames to serial
        USB.println( xbeeDM._payload, xbeeDM._length);
        
      }
      else
      {
        if(error!=1){
          // Print error message:
          /*
           * '7' : Buffer full. Not enough memory space
           * '6' : Error escaping character within payload bytes
           * '5' : Error escaping character in checksum byte
           * '4' : Checksum is not correct	  
           * '3' : Checksum byte is not available	
           * '2' : Frame Type is not valid
           * '1' : Timeout when receiving answer   
          */
          USB.print(F("Error receiving a packet:"));
          USB.println(error,DEC);     
          USB.println(F("--------------------------------"));
        }
      }
      currenttime = millis();
    }
    xbeeDM.OFF();
   }
  }
  else{
    xbeeDM.ON();
    delay(50);
          error = xbeeDM.receivePacketTimeout( 10000 );
    
      // check answer  
      if( error == 0 ) 
      {
        // print frames to serial
        USB.println( xbeeDM._payload, xbeeDM._length);
        
      }
      else
      {
        if(error!=1){
          // Print error message:
          /*
           * '7' : Buffer full. Not enough memory space
           * '6' : Error escaping character within payload bytes
           * '5' : Error escaping character in checksum byte
           * '4' : Checksum is not correct    
           * '3' : Checksum byte is not available 
           * '2' : Frame Type is not valid
           * '1' : Timeout when receiving answer   
          */
          USB.print(F("Error receiving a packet:"));
          USB.println(error,DEC);     
          USB.println(F("--------------------------------"));
        }
      }
    
    }
  
} 

