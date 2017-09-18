/*  
script to receive frames
 */


// 1. Include Libraries
#include <WaspUIO.h>
#include <WaspSensorAgr_v20.h>
#include <WaspFrame.h>
#include <WaspXBeeDM.h>

// define variable
uint8_t error;
int savePower = 0;



void setup()
{  
  // initialize time
  UIO.initTime();
  // Initialize variables, from EEPROM (USB print, OTA programming, ..)
  UIO.initVars();

  // Interactive mode
  UIO.start_RTC_SD_USB(false);
  UIO.menu();

  info(F("*** Booting (setup). Battery level is %d"), UIO.batteryLevel);
  debug(F("Boot done"));
  UIO.stop_RTC_SD_USB();
  RTC.ON();

}



void loop()
{ 
  if(savePower == 1){
  RTC.getTime();
  if((RTC.minute == 58)||(RTC.minute == 18)||(RTC.minute == 38)){

    long starttime = millis();
    long interval = 10 * 1000 * 60;
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
        //if(error!=1){
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
        //}
      }
    
    }
  
} 

