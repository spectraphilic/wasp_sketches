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
#include <WaspXBeeDM.h>
uint8_t error;

void setup()
{
  // put your setup code here, to run once:
  xbeeDM.ON();
  USB.println("xbee on");
  error = xbeeDM.receivePacketTimeout( 10000 );
  USB.println("timeout");
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


void loop()
{
  // put your main code here, to run repeatedly:

}
