/* 
 *  ---  3G_17 - Uploading files to a FTP server from Waspmote's SD   ---
 *
 *  Explanation: This example shows how to upload a file to a FTP server
 *  from Waspmote's SD card.
 *
 *  Copyright (C) 2014 Libelium Comunicaciones Distribuidas S.L.
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
 *  Version:           1.2
 *  Design:            David Gascón
 *  Implementation:    Alejandro Gállego
 */

#include "Wasp3G.h"

char apn[] = "telenor";
char login[] = "login";
char password[] = "password";
/*
char ftp_server[] = "pruebas.libelium.com"; 
char ftp_port[] = "21"; 
char ftp_user_name[] = "t3g@libelium.com"; 
char ftp_password[] = "ftp1234";
*/
char ftp_server[] = "geo-ftp.uio.no";
 char ftp_port[] = "21";
 char ftp_user_name[] = "ftp";
 char ftp_password[] = "johnhh";
 

char filename[] = "/INCOMING/TEST2.TXT"; // Origin filename on SD 
// char filename_ftp[] = "/test2.txt"; // Destination filename on ftp
char filename_ftp[] = "/incoming/johnhh/test2.txt"; // Destination filename on ftp

int answer;

void setup()
{
  USB.println(F("**************************"));
  // 1. sets operator parameters
  _3G.set_APN(apn);
  // And shows them
  _3G.show_APN();
  USB.println(F("**************************"));
}

void loop()
{

  // setup for Serial port over USB:
  USB.println(F("USB port started..."));

  SD.ON();
  USB.println(F("list Root directory:"));
  USB.println(F("---------------------------"));
  SD.ls(LS_R|LS_DATE|LS_SIZE);
  USB.println(F("---------------------------"));

  // 2. activates the 3G module:
  answer = _3G.ON();
  if ((answer == 1) || (answer == -3))
  {
    USB.println(F("3G module ready..."));

    // 3. sets pin code:
    USB.println(F("Setting PIN code..."));
    // **** must be substituted by the SIM code
    if (_3G.setPIN("2409") == 1)
    {
      USB.println(F("PIN code accepted"));
    }
    else
    {
      USB.println(F("PIN code incorrect"));
    }

    // 4. waits for connection to the network
    answer = _3G.check(180);   
    if (answer == 1)
    {
      USB.println(F("3G module connected to the network..."));

      // 5. configures onnection for FTP applications:
      answer = _3G.configureFTP(ftp_server, ftp_port, ftp_user_name, ftp_password, 1, "A");
      if (answer == 1)
      {    


        //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


        _3G.goRoot(0);
        _3G.ls(0);
        USB.println(_3G.buffer_3G);
        _3G.goRoot(1);
        _3G.ls(0);
        USB.println(_3G.buffer_3G);


        // Delete old file from 3G current directory
        answer = _3G.del(strrchr(filename_ftp, '/') + 1);
        if (answer == 1)
        {
          USB.println(F("Old file deleted from 3G module"));
        }
        else
        {
          USB.println(F("Old file NOT deleted from 3G module"));
        }
        //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


        USB.println(F("Uploading the file..."));

        // 6a. uploads file from SD card to the FTP server
        answer = _3G.uploadData(filename, filename_ftp);

        // checks the answer
        if (answer == 1)
        {
          USB.println(F("Upload done"));
        }
        else if(answer == -2)
        {
          USB.print(F("Upload failed. Error code: "));
          USB.println(answer, DEC);
          USB.print(F("CME error code: "));
          USB.println(_3G.CME_CMS_code, DEC);
        }
        else
        {
          USB.print(F("Upload failed. Error code: "));
          USB.println(answer, DEC);
        }
      }
      else
      {
        USB.println(F("Configuration failed. Error code:"));
        USB.println(answer, DEC);
      }
    }
    else
    {
      USB.println(F("3G module cannot connect to the network..."));
    }
  }
  else
  {

    // Problem with the communication with the 3G module
    USB.println(F("3G module not started"));
  }
  // 7. powers off the 3G module
  _3G.OFF();

  USB.println(F("Sleeping..."));

  // 8. sleeps one hour
  PWR.deepSleep("00:01:00:00", RTC_OFFSET, RTC_ALM1_MODE1, ALL_OFF); 
}




