/*  
 *  ------ Waspmote Pro Code Example -------- 
 *  
 *  Explanation: This is the basic Code for Waspmote Pro
 *  
 *  Copyright (C) 2013 Libelium Comunicaciones Distribuidas S.L. 
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
 */

//////////////////////////////////////////
// Included Waspmote Libraries
//////////////////////////////////////////
// #include <WaspXBeeDM.h>
// #include <WaspFrame.h>
// #include <WaspGPS.h>
#include <WaspSensorCities.h>
// #include <WaspStackEEPROM.h>

//////////////////////////////////////////
// User selectable constants
//////////////////////////////////////////

// Define the Waspmote ID ("Mote_1")
//char WASPMOTE_ID[] = "Mote_1";

// Define base station or gateway MAC adress ("0013A20040db6048")
char RX_ADDRESS[] = "0013A20040db6048";

// define GPS timeout when connecting to satellites, this time is defined in seconds (240sec = 4minutes)
#define GPS_TIMEOUT 240

// Set sampling interval for measurements
char sampleRate [] = "00:00:00:05"; // "dd:hh:mm:ss"

// Set interval to log measurements, GPS position and transmitt data to base station
char logRate [] = "00:01:00"; // "dd:hh:mm"

//////////////////////////////////////////
// Global variables declaration
//////////////////////////////////////////

// define variable
//uint8_t tx_error; // Xbee send error
//unsigned long id; // WaspMote serial ID
//bool gps_status; // define status variable for GPS connection
//char* filename="EPHEM.TXT";
//uint8_t sd_answer;  
//char filename[13]; // MUST be 8.3 SHORT FILE NAME
timestamp_t time;

//////////////////////////////////////////
// Module initialization after start or reset
//////////////////////////////////////////
void setup()
{

  USB.ON();
  RTC.ON();


  // Setting time [yy:mm:dd:dow:hh:mm:ss]  !!! OBS get time from GPS or XBEE !!!
  RTC.setTime("15:12:31:04:14:58:52");
  // USB.println(RTC.getTime());



  // Start sampling at next full minute
  RTC.setAlarm1(0,0,0,60-RTC.second,RTC_OFFSET,RTC_ALM1_MODE3);
  USB.println(RTC.getAlarm1());

  // Start logging at next full hour
  RTC.setAlarm2(0,0,60-RTC.minute,RTC_OFFSET,RTC_ALM2_MODE3);
  USB.println(RTC.getAlarm2());
}

//////////////////////////////////////////
// Start program
//////////////////////////////////////////
void loop() 
{
  PWR.sleep(ALL_OFF);

  // After setting Waspmote to power-down, UART is closed, 
  // so it is necessary to open it again
  USB.ON();
  RTC.ON();

  RTC.getTime();
//  USB.println(RTC.hour, DEC);
//  USB.println(RTC.minute, DEC);
//  USB.println(RTC.second, DEC);

  // Check Interruption register, if RTC alarm, sample and/or log data
  if(intFlag & RTC_INT)
  {
    intFlag &= ~(RTC_INT); // Clear flag

    if (RTC.alarmTriggered == 1) 
    {
      RTC.setAlarm1(sampleRate,RTC_OFFSET,RTC_ALM1_MODE3);
      USB.println(RTC.getAlarm1());
    }
    else
    {
      RTC.setAlarm1(sampleRate,RTC_OFFSET,RTC_ALM1_MODE3);
      RTC.setAlarm2(logRate,RTC_OFFSET,RTC_ALM2_MODE3);
      USB.println(RTC.getAlarm1());
      USB.println(RTC.getAlarm2());
    }
  }
  //////////////////////////////////////////
  // Take Samples
  //////////////////////////////////////////



  delay(2000);


  USB.println("tid 2");

  USB.println(RTC.hour, DEC);
  USB.println(RTC.minute, DEC);
  USB.println(RTC.second, DEC);





  if (RTC.alarmTriggered != 1)
  {
    //////////////////////////////////////////
    // log measurements, GPS position 
    // and transmitt data to base station
    //////////////////////////////////////////












  }
}
//////////////////////////////////////////
// End of program
//////////////////////////////////////////





























