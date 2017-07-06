/*  
 *  ------------  WaspMote OTAP testing --------------
 *Locate the node to upgrade.
 *Check current software version.
 *Send the new program.
 *Reboot and start with the new program.
 *Restore the previous program if the process fails. 
 *  
 */


#include <WaspFrame.h>
#include <WaspXBeeDM.h>
#include <WaspUIO.h>

char waspmote_ID[] = "1";
char logfile[] = "";


/*** XBee variables ***/
char RX_ADDRESS[] = "0013a20040db6048"; // "0013a20040779085" Meshlium_Finse; "0013a20040db6048" Gateway MAC

unsigned long epoch;

void setup() 
{
  xbeeDM.ON();

}  

void loop()
{
 
  uint8_t OTA_answer = 0;
  logActivity(logfile, "Check for OTA programing");
  // Check if new data is available
  if( xbeeDM.available() )
  {
    logActivity(logfile, F("check_for_OTA_new_program(): xbee available"));
    USB.println(F("xbee available"));
    xbeeDM.treatData();
    // Keep inside this loop while a new program is being received
    while( xbeeDM.programming_ON  && !xbeeDM.checkOtapTimeout() )
    {
      if( xbeeDM.available() )
      {
        xbeeDM.treatData();
      }
    }
    logActivity(logfile, F("check_for_OTA_new_program(): New program received"));
    USB.println(F("New program received"));
    OTA_answer = 1;
  }
}


//===============================================
//===============================================
//              FUNCTIONS
//===============================================
//===============================================



///////////////////////////////////////////
// Function: Check if any program are uvailable to download via OTA
/////////////////////////////////////////// 

void check_for_OTA_new_program()
{
  // Check if new data is available
  if( xbeeDM.available() )
  {
    xbeeDM.treatData();
    // Keep inside this loop while a new program is being received
    while( xbeeDM.programming_ON  && !xbeeDM.checkOtapTimeout() )
    {
      if( xbeeDM.available() )
      {
        xbeeDM.treatData();
      }
    }
  }
}

















