/*  
 *  ------  3G_10 - TCP client in single connection  -------- 
 *  
 *  Explanation: This example shows how to create a TCP client 
 *  in single connection
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

#include <Wasp3G.h>
#include <WaspFrame.h>

char test_string[]="Test string from Waspmote!\r\n";

char apn[] = "telenor";
char login[] = "telenor";
char password[] = "gprs";

char IP[] = "129.240.118.246";
uint16_t port = 8080;
 
int8_t answer;

void setup()
{
    USB.println(F("**************************"));
    // 1. sets operator parameters
    _3G.set_APN(apn, login, password);
    // And shows them
    _3G.show_APN();
    USB.println(F("**************************"));
}

void loop()
{

    // setup for Serial port over USB:
    USB.ON();
    USB.println(F("USB port started..."));
    USB.println(F("**************************"));

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

            // 5. configures TCP connection
            USB.print(F("Setting connection..."));
            answer = _3G.configureTCP_UDP();
            if (answer == 1)
            {
                USB.println(F("Done"));

                USB.print(F("Opening TCP socket..."));
                // 6. opens a TCP socket
                answer = _3G.createSocket(TCP_CLIENT, IP, port);
                if (answer == 1)
                {
                    USB.println(F("Conected"));
                    if(_3G.getIP() == 1)
                    {
                        // if configuration is success shows the IP address
                        USB.print(F("IP address: ")); 
                        USB.println(_3G.buffer_3G);
                    }


                    //************************************************
                    //             Send a string of text
                    //************************************************

                    USB.print(F("Sending test string..."));
                    // 7. sends 'test_string'
                    answer = _3G.sendData(test_string);
                    if (answer == 1) 
                    {
                        USB.println(F("Done"));
                    }
                    else if (answer == 0)
                    {
                        USB.println(F("Fail"));
                    }
                    else 
                    {
                        USB.print(F("Fail. Error code: "));
                        USB.println(answer, DEC);
                        USB.print(F("CME or IP error code: "));
                        USB.println(_3G.CME_CMS_code, DEC);
                    }


                    //************************************************
                    //             Send a ASCII frame
                    //************************************************

                    USB.print(F("Sending a frame...")); 
                    // create new frame (ASCII)
                    frame.createFrame(ASCII,"Waspmote_Pro"); 
                    // add frame fields
                    frame.addSensor(SENSOR_STR, test_string); 
                    frame.addSensor(SENSOR_BAT, (uint8_t) PWR.getBatteryLevel());
                    RTC.ON();
                    frame.addSensor(SENSOR_TCA, (float) RTC.getTemperature());
                    RTC.OFF();

                    // 8. sends a frame      
                    answer = _3G.sendData(frame.buffer, frame.length);
                    if (answer == 1) 
                    {
                        USB.println(F("Done"));
                    }
                    else if (answer == 0)
                    {
                        USB.println(F("Fail"));
                    }
                    else 
                    {
                        USB.print(F("Fail. Error code: "));
                        USB.println(answer, DEC);
                        USB.print(F("CME or IP error code: "));
                        USB.println(_3G.CME_CMS_code, DEC);
                    }


                    USB.print(F("Closing TCP socket..."));  
                    // 9. closes socket
                    if (_3G.closeSocket() == 1)
                    {
                        USB.println(F("Done"));
                    }
                    else
                    {
                        USB.println(F("Fail"));
                    }
                }
                else if (answer <= -4)
                {
                    USB.print(F("Connection failed. Error code: "));
                    USB.println(answer, DEC);
                    USB.print(F("CME error code: "));
                    USB.println(_3G.CME_CMS_code, DEC);
                }
                else 
                {
                    USB.print(F("Connection failed. Error code: "));
                    USB.println(answer, DEC);
                }           
            }
            else if (answer <= -10)
            {
                USB.print(F("Configuration failed. Error code: "));
                USB.println(answer, DEC);
                USB.print(F("CME error code: "));
                USB.println(_3G.CME_CMS_code, DEC);
            }
            else 
            {
                USB.print(F("Configuration failed. Error code: "));
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

    // 10. Powers off the 3G module
    _3G.OFF();

    USB.println(F("Sleeping..."));

    // 11. sleeps one hour
    PWR.deepSleep("00:01:00:00", RTC_OFFSET, RTC_ALM1_MODE1, ALL_OFF);

}






