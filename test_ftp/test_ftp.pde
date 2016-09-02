/*  
 
 */

#include "Wasp3G.h"

char apn[] = "telenor";
char login[] = "login";
char password[] = "password";



char ftp_server[] = "ftp.uio.no";

//char ftp_server[] = "geo-ftp.uio.no";
char ftp_port[] = "21";
char ftp_user_name[] = "ftp";
char ftp_password[] = "";

int answer;

void setup()
{
  USB.println(F("**************************"));
  // 1. sets operator parameters
  _3G.set_APN(apn);
  //  _3G.set_APN(apn, login, password);
  // And shows them
  _3G.show_APN();
  USB.println(F("**************************"));
}

void loop()
{

  // setup for Serial port over USB:
  USB.ON();
  USB.println(F("USB port started..."));    

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

      // 5. configures connection for FTP applications:
      answer = _3G.configureFTP(ftp_server, ftp_port, ftp_user_name, ftp_password, 1, "I");

      _3G.ls(0);
      USB.println(_3G.buffer_3G);

      if (answer == 1)
      {     
        USB.println(F("Downloading the file..."));

        // 6. downloads file to 3G module (current directory 0) from the FTP server:
        answer = _3G.downloadFile("/welcome.msg", 1, 180 );
        USB.println(_3G.buffer_3G);

        if (answer == 1)
        {
          USB.println(F("Download done"));
          _3G.ls(0);
          USB.println(_3G.buffer_3G);
        }
        else if(answer <= -2)
        {
          USB.print(F("Download failed. Error code: "));
          USB.println(answer, DEC);
          USB.print(F("CME error code: "));
          USB.println(_3G.CME_CMS_code, DEC);
          USB.println(_3G.buffer_3G);
        }
        else 
        {
          USB.print(F("Download failed. Error code: "));
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

