

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
unsigned int counter = 0; //loop counter 
int counter2 = 0; //loop counter 
char buffer[50];

void setup()
{  
  USB.ON();  // open USB port
  RTC.ON();  // set RTC ON

  UIO.initSD();
  UIO.initNet(NETWORK_FINSE);

  // set mote Identifier for GPS sync unit
  frame.setID("GPS_sync");
  GPS.ON();  // set GPS ON
}


void loop()
{
  status = GPS.waitForSignal(TIMEOUT); // OBS, problems with 'loadEphems()' DO NOT USE!!!

  USB.print("status: "); // Debug
  USB.println(status); // Debug


  if (counter<60 || counter2 < 8)
  {
    RTC.setAlarm1("00:00:00:00",RTC_ABSOLUTE,RTC_ALM1_MODE5); // Set alarm to send every full minute
  }
  else
  {
    PWR.deepSleep("00:00:56:00",RTC_ABSOLUTE,RTC_ALM1_MODE4,ALL_OFF); // Set alarm to wake-up  once an hour
    counter2 = 0; // Reset counter2

    counter=60; // reset counter to 60 to force if-statement to false
    UIO.logActivity("Wake-up");

    UIO.delLogActivity();
  }

  if (intFlag & RTC_INT) // RTC captured
  {
    UIO.logActivity("Alarm captured");
    xbeeDM.ON();
    delay(100);

    sprintf(buffer,"Battery: %d%%", PWR.getBatteryLevel());
    UIO.logActivity(buffer);

    // Create Frame depending on the connectivity
    frame.createFrame(ASCII);
    frame.setFrameType(SERVICE1_FRAME); // set a TIMEOUT frame type

    // if GPS is connected then set time from GPS...
    if( status == true )
    {    
      GPS.ON();  // set GPS ON
      // add message fields to Frame
      frame.addSensor(SENSOR_STR,"GPS updated epoch time");
      // set time in RTC from GPS time (GMT time)
      GPS.setTimeFromGPS();
    }
    // ...otherwise, use time from RTC
    else
    {
      // add message fields to Frame
      frame.addSensor(SENSOR_STR,"not updated time");
    }
    // add 'RTC.getEpochTime' timestamp in frame
    frame.addSensor(SENSOR_TST, RTC.getEpochTime());


    // Send GPS_sync message
    error = xbeeDM.send(UIO.BROADCAST_ADDRESS, frame.buffer, frame.length); 
    delay(100);
    xbeeDM.OFF();

    frame.showFrame(); // Debug

    // Check TX flag
    if (error == 0)
    {
      UIO.logActivity("sync sent OK");
    }
    else 
    {
      UIO.logActivity("sync not sent!");
    }

    if(status == false)
    {
      UIO.logActivity("No GPS signal");
      // delay(60000); // Delay one minute before rebooting
      // PWR.reboot(); // It seems to be a bug in the GPS, reboot is needed if no GPS signal
    }
    // Clear interuption flag
    intFlag &= ~(RTC_INT); 
    clearIntFlag(); 
    PWR.clearInterruptionPin();

    counter++; //add to loop counter 
    if(counter>10000)
    {
      counter=61;
    }
    counter2++; //add to loop counter2 

    sprintf(buffer,"c: %d, c2: %d", counter, counter2);
    UIO.logActivity(buffer);
  }
}






























