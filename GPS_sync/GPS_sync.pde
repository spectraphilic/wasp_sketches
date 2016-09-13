

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

// define status variable for GPS connection
bool status;

/*** XBee variables ***/
// Destination MAC address
uint8_t error; // XbeeDM send error

unsigned long gps_sync_time;
unsigned long epoch;


void setup()
{  
  USB.ON();  // open USB port
  GPS.ON();  // set GPS ON  
  RTC.ON();  // set RTC ON

    UIO.initSD();
  UIO.initNet('Finse');

  // set mote Identifier (16-Byte max)
  frame.setID("GPS_sync");
}


void loop()
{
  status = GPS.waitForSignal(TIMEOUT);

  xbeeDM.ON();
  delay(100);

  // Create Frame depending on the connectivity
  frame.createFrame(ASCII);
  frame.setFrameSize(DIGIMESH, BROADCAST, DISABLED, DISABLED);
  frame.setFrameType(SERVICE1_FRAME); // set a TIMEOUT frame type

  // if GPS is connected then get time
  if( status == true )
  {    
    // set time in RTC from GPS time (GMT time)
    GPS.setTimeFromGPS();
    epoch = RTC.getEpochTime();
    gps_sync_time = epoch;
    // add message fields to Frame
    frame.addSensor(SENSOR_STR,"GPS updated epoch time");
    // add 'RTC.getEpochTime' timestamp in frame to avoid delays
    frame.addSensor(SENSOR_TST, RTC.getEpochTime());
  }
  else
  {
    // add message fields to Frame
    frame.addSensor(SENSOR_STR,"not updated epoch time");
    // add 'RTC.getEpochTime' timestamp in frame to avoid delays
    frame.addSensor(SENSOR_TST, RTC.getEpochTime());
    // add 'RTC.getEpochTime' timestamp in frame to avoid delays
    //frame.addSensor(SENSOR_TST, gps_sync_time);
  }

  // Send epoch time message
  error = xbeeDM.send(UIO.BROADCAST_ADDRESS, frame.buffer, frame.length); 
  //error = xbeeDM.send("000000000000FFFF", frame.buffer, frame.length); 

  // 2.3 Check TX flag
  if (error == 0)
  {
    frame.showFrame();
    
    UIO.logActivity("Time sync frame sent OK");
    
        frame.showFrame();

  }
  else 
  {
    UIO.logActivity("Time sync frame not sent!!!");
  }

  if(status == false)
  {
    UIO.logActivity("No GPS signal!!!");

    //    UIO.logActivity("No GPS signal, waspmote reboots!!!");
    //    delay(60000); // Delay one minute before rebooting
    //    PWR.reboot();
  }
}













