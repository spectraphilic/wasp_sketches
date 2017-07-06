/******************************************************************************
 * Includes
 ******************************************************************************/

#ifndef __WPROGRAM_H__
#include <WaspClasses.h>
#endif

#include "WaspUIO_sim.h"
#include <stdio.h>
#include <WaspXBeeDM.h>
#include <WaspUSB.h>


/******************************************************************************
* Constructors 
******************************************************************************/
//UIO::UIO()
//{
		
//}

/******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/
 

uint8_t WaspUIO::check_for_connection_to_Meshlium(char logfile)
{
  uint8_t error = 0;
  uint8_t answer = 0;

  if( xbeeDM.available() )
  {
    error = xbeeDM.receivePacketTimeout( 10000 );
    if( error == 0 ) 
    {
      answer = 0;
      // Show data stored in '_payload' buffer indicated by '_length'
      USB.print(F("--> Data: "));  
      USB.println( xbeeDM._payload, xbeeDM._length);
      
      // Show data stored in '_payload' buffer indicated by '_length'
      USB.print(F("--> Length: "));  
      USB.println( xbeeDM._length,DEC);
      
      // Show data stored in '_payload' buffer indicated by '_length'
      USB.print(F("--> Source MAC address: "));      
      USB.printHex( xbeeDM._srcMAC[0] );    
      USB.printHex( xbeeDM._srcMAC[1] );    
      USB.printHex( xbeeDM._srcMAC[2] );    
      USB.printHex( xbeeDM._srcMAC[3] );    
      USB.printHex( xbeeDM._srcMAC[4] );    
      USB.printHex( xbeeDM._srcMAC[5] );    
      USB.printHex( xbeeDM._srcMAC[6] );    
      USB.printHex( xbeeDM._srcMAC[7] );    
      USB.println();

      USB.print(F("Packet successfully received:"));
      logActivity(logfile, F("xbeeDM payload:"));
      logActivity(logfile, xbeeDM._payload);
      logActivity(logfile, error);  

    }
    else
    {
      answer = 1;
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
      USB.println(error);
      logActivity(logfile, F("Error receiving a packet:"));
      logActivity(logfile, error);     
    }
  
  // wait for 5 seconds
  USB.println(F("\n----------------------------------"));
  delay(5000);

  }
  return answer;
}




 /******************************************************************************/
 /* Function to synchronize Waspmote time with Meshlium
 *  Parameters:   int* RX_ADRESS   - MAC address of the Meshlium device (@_Finse:0013a20040779085)
 *                char* logfile   - Filename of the activity log text file
 * 
 * Returns: uint8_t   - 1 on success, 0 for error
 *
 *WARNINGS: Unstable setRTCfromMeshlium() function with meshlium version
 */

uint8_t WaspUIO::synchronize_time_from_meshlium(int RX_ADDRESS, char logfile)
{
  if( xbeeDM.available() )
  {
    uint8_t answer = 0;
    logActivity(logfile, F("Setup: Set time from Meslium"));
    USB.println(F("Setup: Set time from Meslium"));
    delay(500);
    xbeeDM.setRTCfromMeshlium(RX_ADDRESS);
    delay(500);
    logActivity(logfile, F("Setup: time updated: "));
    USB.print(F("Setup: time updated: "));
    USB.println(RTC.getTime());
    xbeeDM.OFF();
    answer = 1;
  }
  return answer;
}

 /******************************************************************************/
 /* Function to receive new program through OTA 
 *  Parameters: char* logfile   - Filename of the activity log text file
 * 
 * Returns: uint8_t   - 1 on success, 0 for error
 */

uint8_t WaspUIO::check_for_OTA_new_program(char logfile)
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
  return OTA_answer;
}


 /******************************************************************************/
 /* Function to log waspmote activity
 * 
 * Parameters: char* filename   - Filename to log activity
 *             char* message    - Message to log in filename
 *
 * Returns: uint8_t             - 1 on success, 0 for error
 */
 
 uint8_t WaspUIO::logActivity( char filename[], char message[] )
{
  int answer = 0;

  // Make sure a file called filename exists before writing to SD
  if(SD.isFile(filename) == 1)
  {
    SD.append(filename, RTC.getTime());
    SD.append(filename, "\t");
    answer = SD.appendln(filename, message);
    delay(100);
  }
  return answer;
}

 /******************************************************************************/
 /* Function to to store a frame on SD card 
 * 
 * Parameters: char* filename   - Filename to stor data
 * 
 * Returns: uint8_t             - 1 on success, 0 for error
 */
 
 uint8_t WaspUIO::Frame2SdFile( char* filename, char* frame )
{
  //char path[]="/data";
  //char filename[]="/data/log.txt";
  // define folder and file to store data
  uint8_t sd_answer;
  SD.create(filename);
  sd_answer = SD.appendln(filename, frame.buffer, frame.length);
  delay(100);

  #if DEBUG_UIO
     if(sd_answer == 1)
  {	  
    USB.println(F("Frame appended to file"));
  }
  else 
  {
    USB.println(F("Append failed"));
  }
	 #endif
	 
  return sd_answer;
}

 /******************************************************************************/
 /* Function to to send a frame with xBeeDM and store the frame on permanent file inSD 
 *  card. If connection not working, save frame to filename_unsent
 * 
 * Parameters: char* filename_tmp     - Filename where the frames are located. WARNINGS: this file will be DELETED!!!!!
 *             char* filename_final   - Filename where the frames are permanently located. 
 *             char* filename_usnent  - Filename where the frames unsent are moved.
               char* logfile          - Filename of the log activity text file
 *             int32_t* RX_ADDRESS    - MAC address of meshlium
 *          
 * Returns: uint8_t             - 1 on success, 0 for error
 */
 
 uint8_t WaspUIO::Frame2Meshlium( char* filename_tmp, char* filename_final, char* filename_unsent, int32_t* RX_ADDRESS, char* logfile )
 {
  //char path[]="/data";
  //char filename[]="/data/log.txt";
  // define folder and file to store data
  uint8_t sd_answer;
  uint8_t connect_success;
  uint8_t frame_saved;
  uint8_t frameSD[MAX_FRAME+1];



  // 1. read frame from tmp.txt file
  int32_t numLines_tmp = SD.numln(filename_tmp);
  int32_t numLines_usnent = SD.numln(filename_unsent);

  // Only perform the function if there two or more lines in temporary file
  if(numLines_tmp >= 2)
  {
    for( int i=0; i<numLines_tmp ; i++ )
    {
      SD.catln( filename_data, i, 1);
      memset(frameSD, 0x00, sizeof(frameSD) );
      lengthSD = Utils.str2hex(SD.buffer, frameSD );
      logActivity(logfile, F("Frame2Meshlium(): Read frame from temporary data file"));


  //2. send frame to netword
      connect_success = xbeeDM.send(RX_ADDRESS, frameSD, lengthSD);
      delay(1000);

      if(connect_success == 1)
      {
        logActivity(logfile, F("Frame2Meshlium(): Frame sent through network to Meshlium"));
    //2.1 write frame to permanent data file
        frame_saved = Frame2SdFile(filename_final, frameSD);

        if(frame_saved == 1){
          logActivity(logfile, F("Frame2Meshlium(): Frame saved to permanent data file and deleted from temporary"));
        }
        else{

          logActivity(logfile, F("Frame2Meshlium(): Frame not saved to permanent"));
        }

      }
      else{
        // if frame is not sent, then it is stored to 
        Frame2SdFile(filename_unsent, frameSD);
        logActivity(logfile, F("Frame2Meshlium(): Frame not sent, moved to unsent datafile"));
      }

    }
    SD.del(filename_tmp);
    sd_answer = 1;
    logActivity(logfile, F("Frame2Meshlium(): temporary file deleted"));
  }
  else
  {
    logActivity(logfile, F("Frame2Meshlium(): Not enough data in temporary file"));
    sd_answer = 0;
  }
  return sd_answer;
}

 

