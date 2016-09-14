/*  
 *  ------ [DM_03] - receive a  packet -------- 
 *  
 *  Explanation: This example shows how to receive packets with 
 *  XBee modules
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

#include <WaspFrame.h>
#include <WaspXBeeDM.h>
#include <WaspUIO.h>


// define variable
uint8_t error;
char buffer[100];
char * pch;
unsigned long epoch; // Define variable for epoch time
timestamp_t time; // Define variable for UTC timestamps
char * pEnd;

void setup()
{  
  // init USB port
  USB.ON();
  USB.println(F("Receiving example"));

  UIO.initSD();
  UIO.initNet('Finse');

  // init XBee 
  xbeeDM.ON();
}


void loop()
{ 
  // receive XBee packet (wait for 5 minutes)
  error = xbeeDM.receivePacketTimeout(300000);


  // check answer  
  if(error == 0) 
  {
    // Show data stored in '_payload' buffer indicated by '_length'
    USB.println( xbeeDM._payload, xbeeDM._length); // For debug!!!

    // clear buffers
    memset(buffer, 0x00, sizeof(buffer));
    // copy _payload to buffer
    memcpy(buffer, xbeeDM._payload, xbeeDM._length);

    if (strstr(buffer,"GPS_sync") != 0) // check that this frame commes from the 'GPS_sync' unit
    {
      pch = strstr(buffer,"TST:");
      USB.println(pch); // For debug!!!
      //----------------------------------------
      // Simon, you are free to do want you want here! ;)


      // Gave up here!!! Find solution to convert "TST:1473853802#" -> 1473853802   !!!!



      //----------------------------------------
      // epoch = strtol (pch,&pEnd,10);

      // epoch = atol("1473849722"); // For debug
      // epoch = epoch + 2; // Add some seconds for the delays, Check this!!!
      USB.println (epoch);

      // Break Epoch time into UTC time
      RTC.breakTimeAbsolute( epoch, &time );
      // Available info: UTC Date
      USB.print( time.year, DEC );
      USB.print( time.month, DEC );
      USB.print( time.date, DEC );
      USB.print( time.hour, DEC );
      USB.print( time.minute, DEC );
      USB.print( time.second, DEC );

    }
    else
    {
      USB.println ("Frame resived from other than 'GPS_sync' unit. Ignore!!!");
    }
  }
  else
  {
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
  }
} 














