
/*  
 *  ------ Waspmote Pro Code Example -------- 
 */

// Put your libraries here (#include ...)
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
char* RX_ADDRESS="000000000000FFFF";
uint8_t error; // XbeeDM send error

unsigned long gps_sync_time;
unsigned long epoch;


void setup()
{  
  USB.ON();  // open USB port
  GPS.ON();  // set GPS ON  
  RTC.ON();  // set RTC ON

    // set mote Identifier (16-Byte max)
  frame.setID("GPS_sync");

}


void loop()
{
  status = GPS.waitForSignal(TIMEOUT);

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
    delay(100);
  }

  // add error message fields to Frame
  frame.addSensor(SENSOR_STR,"Epoch/GPS sync time");
  // add 'RTC.getEpochTime' timestamp in frame to avoid delays
  frame.addSensor(SENSOR_TST, RTC.getEpochTime());
  // add 'RTC.getEpochTime' timestamp in frame to avoid delays
  frame.addSensor(SENSOR_TST, gps_sync_time);

  frame.showFrame();

  // Send epoch time message
  xbeeDM.ON(); 
  delay(2000); 

  // Initial message transmission
  error = xbeeDM.send( RX_ADDRESS, frame.buffer, frame.length ); 

  // 2.3 Check TX flag
  if ( error == 0 ) 
  {
    USB.println(F("ok"));
  }
  else 
  {
    USB.println(F("error"));
  }

  // 2.4 Communication module to OFF
  xbeeDM.OFF();
  delay(100);

  if(error==1 || status==false)
  {
    PWR.reboot();
  }


  delay(5000);

  USB.println(F("----------------------"));
}



















