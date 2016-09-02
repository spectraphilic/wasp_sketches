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

// Put your libraries here (#include ...)
#include <WaspXBeeDM.h>



void setup() 

{
  UIOinit('Finse','AWS');
}


void loop() 
{
  // put your main code here, to run repeatedly:
}


///////////////////////////////////////////
// Function: UIO:init
/////////////////////////////////////////// 
void UIOinit(int network, int moteType)
{
  char message[100];
  char* logfile = "log.txt";


  // 1) Check the SD card and Log file 
  USB.ON();
  if (SD.isSD()==0) //Check for SD card
  {
    USB.println(F("Warning: No SD card installed"));
    // breake
  }
  if(SD.isFile(logfile)!=1) //Check for log file
  {
    if(SD.create(logfile))
    {
      USB.println(F("Log file created"));
    }
    else
    {
      USB.print(F("Warning: Log file NOT created, error: "));
      USB.println(SD.flag);
    }  
  }




  // 2) Configure the mote for a network 
  uint8_t  panID[2]; 
  uint8_t  channel;
  uint8_t encryptionMode;
  char  encryptionKey[] = "UIO!";

  switch(network)
  {
  case 'Finse': // -----------Settings for Finse WSN --------------------------
    USB.println(F("Configure for Finse WSN"));

    panID[0] = 0x12;
    panID[1] = 0x34;
    channel = 0x0F;
    encryptionMode = 0;
    break;

  case 'Other':
    USB.println(F("Configure for other WSN"));
    break;


  default :
    USB.println(F("No configuration"));
  }


  // open USB port
  USB.ON();

  USB.println(F("-------------------------------"));
  USB.println(F("Configure XBee Digimesh"));
  USB.println(F("-------------------------------"));


  // init XBee 
  xbeeDM.ON();    

  /////////////////////////////////////
  // 1. set channel 
  /////////////////////////////////////
  xbeeDM.setChannel( channel );

  // check at commmand execution flag
  if( xbeeDM.error_AT == 0 ) 
  {
    USB.print(F("1. Channel set OK to: 0x"));
    USB.printHex( xbeeDM.channel );
    USB.println();
  }
  else 
  {
    USB.println(F("1. Error calling 'setChannel()'"));
  }


  /////////////////////////////////////
  // 2. set PANID
  /////////////////////////////////////
  xbeeDM.setPAN( panID );

  // check the AT commmand execution flag
  if( xbeeDM.error_AT == 0 ) 
  {
    USB.print(F("2. PAN ID set OK to: 0x"));
    USB.printHex( xbeeDM.PAN_ID[0] ); 
    USB.printHex( xbeeDM.PAN_ID[1] ); 
    USB.println();
  }
  else 
  {
    USB.println(F("2. Error calling 'setPAN()'"));  
  }

  /////////////////////////////////////
  // 3. set encryption mode (1:enable; 0:disable)
  /////////////////////////////////////
  xbeeDM.setEncryptionMode( encryptionMode );

  // check the AT commmand execution flag
  if( xbeeDM.error_AT == 0 ) 
  {
    USB.print(F("3. AES encryption configured (1:enabled; 0:disabled):"));
    USB.println( xbeeDM.encryptMode, DEC );
  }
  else 
  {
    USB.println(F("3. Error calling 'setEncryptionMode()'"));
  }

  /////////////////////////////////////
  // 4. set encryption key
  /////////////////////////////////////
  xbeeDM.setLinkKey( encryptionKey );

  // check the AT commmand execution flag
  if( xbeeDM.error_AT == 0 ) 
  {
    USB.println(F("4. AES encryption key set OK"));
  }
  else 
  {
    USB.println(F("4. Error calling 'setLinkKey()'")); 
  }

  /////////////////////////////////////
  // 5. write values to XBee module memory
  /////////////////////////////////////
  xbeeDM.writeValues();

  // check the AT commmand execution flag
  if( xbeeDM.error_AT == 0 ) 
  {
    USB.println(F("5. Changes stored OK"));
  }
  else 
  {
    USB.println(F("5. Error calling 'writeValues()'"));   
  }

  USB.println(F("-------------------------------")); 




  /////////////////////////////////////
  // 1. get channel 
  /////////////////////////////////////
  xbeeDM.getChannel();
  USB.print(F("channel: "));
  USB.printHex(xbeeDM.channel);
  USB.println();

  /////////////////////////////////////
  // 2. get PAN ID
  /////////////////////////////////////
  xbeeDM.getPAN();
  USB.print(F("panid: "));
  USB.printHex(xbeeDM.PAN_ID[0]); 
  USB.printHex(xbeeDM.PAN_ID[1]); 
  USB.println(); 

  /////////////////////////////////////
  // 3. get Encryption mode (1:enable; 0:disable)
  /////////////////////////////////////
  xbeeDM.getEncryptionMode();
  USB.print(F("encryption mode: "));
  USB.printHex(xbeeDM.encryptMode);
  USB.println(); 

  USB.println(F("-------------------------------")); 

  delay(100);



  // 3) Setup the sensors beeing used (moteType)
  switch(moteType)
  {
  case 'AWS':
    USB.println(F("Configure mote with AWS sensors"));



    break;





  case 'Gage':
    USB.println(F("Configure mote with Gage sensors"));
    break;


  default :
    USB.println(F("Configure mote with standard sensors"));
  }

  // 4) Update time 


  // 5) Print information to log file and USB



  // 6) Print log.txt to usb

  SD.showFile(logfile);




  USB.OFF();

}


































