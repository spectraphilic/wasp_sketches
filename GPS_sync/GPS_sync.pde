

/*  
 *  ------ Waspmote Pro Code Example -------- 
 */

// Put your libraries here (#include ...)
#include <WaspUIO.h>
#include <WaspGPS.h>
#include <WaspXBeeDM.h>
#include <WaspFrame.h>

// define GPS timeout when connecting to satellites
// this time is defined in seconds (240sec = 4minutes)
#define TIMEOUT 240

bool status; // define status variable for GPS connection
uint8_t error; // XbeeDM send error

void setup()
{  
  USB.ON();  // open USB port
  RTC.ON();  // set RTC ON

  UIO.initSD();
  UIO.initNet('Finse');

  // set mote Identifier for GPS sync unit
  frame.setID("GPS_sync");
  GPS.ON();  // set GPS ON
}


void loop()
{
  RTC.setAlarm1("00:00:00:00",RTC_ABSOLUTE,RTC_ALM1_MODE5); // Set alarm to send every full minute

  status = GPS.waitForSignal(TIMEOUT); // OBS, problems with 'loadEphems()' DO NOT USE!!!

  if (intFlag & RTC_INT) // RTC captured
  {
    xbeeDM.ON();
    delay(100);

    // Create Frame depending on the connectivity
    frame.createFrame(ASCII);
    frame.setFrameType(SERVICE1_FRAME); // set a TIMEOUT frame type

    // if GPS is connected then set time from GPS...
    if( status == true )
    {    
      // add message fields to Frame
      frame.addSensor(SENSOR_STR,"GPS updated epoch time");
      // set time in RTC from GPS time (GMT time)
      GPS.setTimeFromGPS();
    }
    // ...otherwise, use time from RTC
    else
    {
      // add message fields to Frame
      frame.addSensor(SENSOR_STR,"not updated epoch time");
    }
    // add 'RTC.getEpochTime' timestamp in frame
    frame.addSensor(SENSOR_TST, RTC.getEpochTime());


    // Send GPS_sync message
    error = xbeeDM.send(UIO.BROADCAST_ADDRESS, frame.buffer, frame.length); 
    delay(100);
    xbeeDM.OFF();

    // Check TX flag
    if (error == 0)
    {
      UIO.logActivity("Time sync frame sent OK");
    }
    else 
    {
      UIO.logActivity("Time sync frame not sent!!!");
    }

    if(status == false)
    {
      UIO.logActivity("No GPS signal, GPS_sync unit reboots!!!");
      delay(60000); // Delay one minute before rebooting
      PWR.reboot(); // It seems to be a bug in the GPS, reboot is needed if no GPS signal
    }
    // Clear interuption flag
    intFlag &= ~(RTC_INT); 
    clearIntFlag(); 
    PWR.clearInterruptionPin();
  }
}






