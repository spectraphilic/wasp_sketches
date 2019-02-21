/*
 *  ------ [GPS_01] - Getting basic data from GPS module --------
 *
 *  Explanation: Set GPS module ON. Wait until it is connected to
 *  the satellites. Then information is requested. All data is stored
 *  in GPS attributes. Finally, a conversion to degrees is made in order
 *  to make easy to search positions in internet GPS websites.
 *
 *  Copyright (C) 2017 Libelium Comunicaciones Distribuidas S.L.
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
 *  Version:           3.0
 *  Design:            David Gascón
 *  Implementation:    Eduardo Hernando
 */

#include <WaspGPS.h>

// define GPS timeout when connecting to satellites
// this time is defined in seconds (240sec = 4minutes)
#define TIMEOUT 240

// define status variable for GPS connection
bool status;


void setup()
{
  // Open USB port
  USB.ON();
  USB.println(F("GPS_01 example"));

  // Set GPS ON  
  GPS.ON();  
}




void loop()
{
  ///////////////////////////////////////////////////
  // 1. wait for GPS signal for specific time
  ///////////////////////////////////////////////////
  status = GPS.waitForSignal(TIMEOUT);
  
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

  ///////////////////////////////////////////////////
  // 2. if GPS is connected then get position
  ///////////////////////////////////////////////////
  if( status == true )
  {
    USB.println(F("\nGET POSITION:"));
    Utils.blinkRedLED(200, 5);
    
    // getPosition function gets all basic data 
    GPS.getPosition();   

    // Time
    USB.print(F("Time [hhmmss.sss]: "));
    USB.println(GPS.timeGPS);

    // Date
    USB.print(F("Date [ddmmyy]: "));
    USB.println(GPS.dateGPS);

    // Latitude
    USB.print(F("Latitude [ddmm.mmmm]: "));
    USB.println(GPS.latitude);  
    USB.print(F("North/South indicator: "));
    USB.println(GPS.NS_indicator);

    //Longitude
    USB.print(F("Longitude [dddmm.mmmm]: "));
    USB.println(GPS.longitude);  
    USB.print(F("East/West indicator: "));
    USB.println(GPS.EW_indicator);

    // Altitude
    USB.print(F("Altitude [m]: "));
    USB.println(GPS.altitude);

    // Speed
    USB.print(F("Speed [km/h]: "));
    USB.println(GPS.speed);

    // Course
    USB.print(F("Course [degrees]: "));
    USB.println(GPS.course);    


    USB.println("\nCONVERSION TO DEGREES (USEFUL FOR INTERNET SEARCH):");
    USB.print("Latitude (degrees):");
    USB.println(GPS.convert2Degrees(GPS.latitude, GPS.NS_indicator));
    USB.print("Longitude (degrees):");
    USB.println(GPS.convert2Degrees(GPS.longitude, GPS.EW_indicator));

  }
  delay(5000);


}

