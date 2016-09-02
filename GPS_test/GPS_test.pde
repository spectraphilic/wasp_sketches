/*  
 *  ------ [GPS_05] - Send GPS data with XBee-Digimesh module -------- 
 *  
 *  Explanation: Set GPS module ON. Wait until it is connected to
 *  the satellites. Depending on the connectivity, a frame with GPS 
 *  info or error message is prepared and sent through XBee-Digimesh
 *  module in broadcast mode.
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

#include <WaspGPS.h>
#include <WaspXBeeDM.h>
#include <WaspFrame.h>


/*** GPS variables ***/

// define GPS timeout when connecting to satellites
// this time is defined in seconds (240sec = 4minutes)
#define TIMEOUT 240

// define status variable for GPS connection
bool status;

char waspID[13] = "WASPMOTE_GPS";
char GPSmsg[] = "GPS not connected";
/*** XBee variables ***/

// Destination MAC address
char MAC_ADDRESS[17]="0013A20040db6048";

//Pointer an XBee packet structure 
packetXBee* packet; 




void setup()
{
  // Open USB port
  USB.ON();
  USB.println(F("GPS_05 example"));

  // init XBee
  xbeeDM.ON();

  // Set GPS ON  
  GPS.ON();  

  // set mote Identifier (16-Byte max)
  frame.setID(waspID);	
}




void loop()
{  

  ///////////////////////////////////////////////
  // 1. Wait for GPS signal
  ///////////////////////////////////////////////
  status = GPS.waitForSignal(TIMEOUT);
    
  // wait for GPS signal for specific time
  if( status == true )
  {
    USB.println(F("\n----------------------"));
    USB.println(F("Connected"));
    USB.println(F("----------------------"));
  }
  else
  {
    USB.println(F("\n----------------------"));
    USB.println(F("GPS TIMEOUT. NOT connected"));
    USB.println(F("----------------------"));
  }


  ///////////////////////////////////////////////
  // 2. Create Frame depending on the connectivity
  ///////////////////////////////////////////////

  // create new frame
  frame.createFrame(ASCII);  
  
  // if GPS is connected then get position
  if( status == true )
  {
    // getPosition function gets all basic data 
    GPS.getPosition();   

    USB.print("Latitude (degrees):");
    USB.println(GPS.convert2Degrees(GPS.latitude, GPS.NS_indicator));
    USB.print("Longitude (degrees):");
    USB.println(GPS.convert2Degrees(GPS.longitude, GPS.EW_indicator));

    // add frame fields
    frame.addSensor(SENSOR_GPS, 
                    GPS.convert2Degrees(GPS.latitude, GPS.NS_indicator),
                    GPS.convert2Degrees(GPS.longitude, GPS.EW_indicator) );
  }
  else
  {    
    // add frame fields
    frame.addSensor(SENSOR_STR, GPSmsg);
  }


  ///////////////////////////////////////////////
  // 3. Send packet
  ///////////////////////////////////////////////

  // Set parameters to packet:
  packet=(packetXBee*) calloc(1,sizeof(packetXBee)); // Memory allocation
  packet->mode=BROADCAST; // Choose transmission mode: UNICAST or BROADCAST

  // Set destination XBee parameters to packet
  xbeeDM.setDestinationParams(packet, MAC_ADDRESS, frame.buffer, frame.length); 

  // send XBee packet
  xbeeDM.sendXBee(packet);
  
  // check TX flag
  if( xbeeDM.error_TX == 0 )
  {
    USB.println(F("send ok"));
  }
  else
  {
    USB.println(F("error"));
  }

  // free memory
  free(packet);
  packet=NULL;


  delay(5000);


}
