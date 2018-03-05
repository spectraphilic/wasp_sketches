/*
    ------ Waspmote factory default code ------

    Explanation: Send basic parameters through the corresponding
    radio module.

    Copyright (C) 2017 Libelium Comunicaciones Distribuidas S.L.
    http://www.libelium.com

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    Version:         3.2
    Design:          David Gascón
    Implementation:  Yuri Carmona, Javier Siscart
*/

// Include libraries
#include <WaspXBee802.h>
#include <WaspXBeeZB.h>
#include <WaspFrame.h>


// Define the Waspmote default ID
char node_id[] = "node_id";

// Define the authentication key
char key_access[] = "LIBELIUM";

// Declare global variables
char macHigh[11];
char macLow[11];
char filename[] = "TEST_SD.TXT";
uint8_t lora_error;
uint8_t lora_status;
uint8_t xbee_error;
uint8_t response;

// Broadcast address
char destination[] = "000000000000FFFF";

// Declare the XBEE_type
// 0 - NO radio detected
// 1 - 802.15.4
// 2 - 900 MHz - 868 MHz
// 3 - DigiMesh
// 4 - XBee ZB
// 5 - 900 MHz Intl
// 6 - LoRa (deprecated)
// 7 - LoRaWAN
// 8 - Sigfox
// 9 - WiFi PRO
// 10 - 4G
int radio_type = 3  ;

// Declare the xbee type in the case there is an XBee module
uint8_t firmware[4];



void setup()
{
  USB.ON();
  USB.println(F("Starting Waspmote factory default program"));


  ///////////////////////////////////////////////////////////////////
  // 1. Serial ID
  ///////////////////////////////////////////////////////////////////

  // Show '_serial_id' stored by the API when powering up
  USB.print(F("Global variable '_serial_id':"));
  USB.printHex(_serial_id[0]);
  USB.printHex(_serial_id[1]);
  USB.printHex(_serial_id[2]);
  USB.printHex(_serial_id[3]);
  USB.printHex(_serial_id[4]);
  USB.printHex(_serial_id[5]);
  USB.printHex(_serial_id[6]);
  USB.printHex(_serial_id[7]);
  USB.println();

  // Reading the serial number
  Utils.readSerialID();
  USB.print(F("Waspmote serial ID: "));
  USB.printHex(_serial_id[0]);
  USB.printHex(_serial_id[1]);
  USB.printHex(_serial_id[2]);
  USB.printHex(_serial_id[3]);
  USB.printHex(_serial_id[4]);
  USB.printHex(_serial_id[5]);
  USB.printHex(_serial_id[6]);
  USB.printHex(_serial_id[7]);
  USB.println();


  ///////////////////////////////////////////////////////////////////
  // 2. Boards ON
  ///////////////////////////////////////////////////////////////////
  PWR.setSensorPower(SENS_3V3, SENS_ON);


  /////////////////////////////////////////////////////////////
  // 3. Test SD
  /////////////////////////////////////////////////////////////
  SD.ON();
  if (SD.isSD())
  {
    SD.del(filename);
    SD.create(filename);
    if (SD.writeSD(filename, "Test SD", 0))
    {
      USB.println(F("SD OK"));
    }
    else
    {
      USB.println(F("Error SD"));
    }
  }
  else
  {
    USB.println(F("No SD card detected"));
  }
  SD.del(filename);
  USB.println();


  /////////////////////////////////////////////////////////////
  // 4. Store key access in EEPROM
  /////////////////////////////////////////////////////////////
  Utils.setAuthKey(key_access);


  /////////////////////////////////////////////////////////////
  // 5. Init RTC and ACC
  /////////////////////////////////////////////////////////////
  RTC.ON();
  ACC.ON();


  /////////////////////////////////////////////////////////////
  // 6. Set Waspmote setting for XBee module for first time.
  // (baudrate at 115200 and API mode enabled)
  /////////////////////////////////////////////////////////////
  // Note: Only valid for SOCKET 0
  xbee802.ON();
  Utils.setMuxSocket0();
  delay(500);
  beginSerial(9600, 0);
  printString("+++", 0);
  delay(2000);
  printString("ATBD7,AP2,WR,CN\r\n", 0);
  delay(500);

  xbeeZB.OFF();
  delay(500);
  xbeeZB.ON();

  // In case of Zigbee modules:
  // XBee command for 115200bps --> ATBD7
  uint8_t ATBD7[] = { 0x7E, 0x00, 0x05, 0x08, 0x01, 0x42, 0x44, 0x07, 0x69 };
  // XBee command for API mode --> ATAP2
  uint8_t ATAP2[] = { 0x7E, 0x00, 0x05, 0x08, 0x01, 0x41, 0x50, 0x02, 0x63 };
  // XBee command for saving config --> ATWR
  uint8_t ATWR[] = { 0x7E, 0x00, 0x04, 0x08, 0x01, 0x57, 0x52, 0x4D };

  for (uint8_t i = 0; i < 9; i++)
  {
    printByte(ATBD7[i], SOCKET0);
  }
  delay(150);
  closeSerial(SOCKET0);
  delay(200);
  beginSerial(115200, SOCKET0);
  for (uint8_t i = 0; i < 9; i++)
  {
    printByte(ATAP2[i], SOCKET0);
  }
  delay(150);
  for (uint8_t i = 0; i < 8; i++)
  {
    printByte(ATWR[i], SOCKET0);
  }
  delay(150);
  closeSerial(SOCKET0);


  /////////////////////////////////////////////////////////////
  // 7. LEDs management
  /////////////////////////////////////////////////////////////
  Utils.setLED(LED0, LED_OFF);
  Utils.setLED(LED1, LED_OFF);
  for (int i = 0 ; i < 4 ; i++)
  {
    Utils.blinkLEDs(100);
  }


  /////////////////////////////////////////////////////////////
  // 8. Identify the radio connected to Waspmote
  //
  //  Possibilities:
  //   - XBee 802.15.4
  //   - XBee 868LP
  //   - XBee 900HP
  //   - XBee DigiMesh
  //   - XBee ZigBee
  //   - LoRa (SX1272) (deprecated)
  //   - LoRaWAN (RN2483 or RN2903)
  //   - Sigfox (TD1207 or TD1508)
  //   - WiFi PRO
  //   - 4G (in SOCKET1)
  /////////////////////////////////////////////////////////////
  uint8_t answer;
  radio_type = 0;



  /////////////////////////////////////////////////////////////
  // 8.1. check for XBee module
  /////////////////////////////////////////////////////////////

  // define object for UART0
  WaspUART uart = WaspUART();

  // init object in SOCKET0
  uart.setUART(SOCKET0);

  // select multiplexer
  Utils.setMuxSocket0();

  // begin serial communication
  uart.beginUART();

  // power on the socket
  PWR.powerSocket(SOCKET0, HIGH);
  delay(500);
  serialFlush(SOCKET0);

  // check for XBees in SOCKET0
  uint8_t cmd_xbee[] = {0x7E, 0x00 , 0x04 , 0x08 , 0x01 , 0x56 , 0x52 , 0x4E};

  // send command & receive answer
  uart.sendCommand(cmd_xbee, sizeof(cmd_xbee));
  uart.readBuffer(100);

  // check response: 7E00078801565200xxxx??
  if (uart._length > 0)
  {
    if ((uart._buffer[0] == 0x7E)
        &&  (uart._buffer[1] == 0x00)
        &&  (uart._buffer[3] == 0x88)
        &&  (uart._buffer[4] == 0x01)
        &&  (uart._buffer[5] == 0x56)
        &&  (uart._buffer[6] == 0x52)
        &&  (uart._buffer[7] == 0x00))
    {

      USB.println(F("XBee module is plugged on socket 0:"));

      /*
        USB.print(F("XBee module in SOCKET0. Firmware: "));
        USB.printHex(uart._buffer[8]);
        USB.printHex(uart._buffer[9]);
        USB.println();
      */

      firmware[0] = uart._buffer[8];
      firmware[1] = uart._buffer[9];
      firmware[2] = uart._buffer[10];
      firmware[3] = uart._buffer[11];

      /////////////////////////////////
      // Get the XBee firmware version
      /////////////////////////////////

      // Set the XBee firmware type depending ont he previous response
      if ((firmware[0] < 0x20) && (firmware[1] > 0x80))
      {
        radio_type = 1; // 802.15.4
        USB.println(F("--> XBee type: 802.15.4"));
      }
      else if ((firmware[0] < 0x20) && (firmware[1] > 0x00))
      {
        radio_type = 2; // 868MHz - 900MHz

        xbee802.OFF();
        xbee802.ON();

        // send Hardware Serial command
        if (!xbee802.sendCommandAT("HS#"))
        {
          // USB.printHexln(xbee802.commandAT, 3);
        }

        // check for XBee 900HP
        if (xbee802.commandAT[0] == 3)
        {
          USB.println(F("--> XBee type: 900HP"));

          // check for available frequencies:
          // US: 00FFFFFFFFFFFFFFFF
          // BR: 00FFFFFFFE00000FFF
          // AU: 00FFFFFFFE00000000
          if (!xbee802.sendCommandAT("AF#"))
          {
            //USB.printHexln(xbee802.commandAT, 16);
            if ((xbee802.commandAT[1] == 0xFF)
                && (xbee802.commandAT[2] == 0xFF)
                && (xbee802.commandAT[3] == 0xFF)
                && (xbee802.commandAT[4] == 0xFF)
                && (xbee802.commandAT[5] == 0xFF)
                && (xbee802.commandAT[6] == 0xFF)
                && (xbee802.commandAT[7] == 0xFF)
                && (xbee802.commandAT[8] == 0xFF))
            {
              USB.println(F("--> Hardware serie: USA"));
            }
            else if ((xbee802.commandAT[1] == 0xFF)
                     && (xbee802.commandAT[2] == 0xFF)
                     && (xbee802.commandAT[3] == 0xFF)
                     && (xbee802.commandAT[4] == 0xFE)
                     && (xbee802.commandAT[5] == 0x00)
                     && (xbee802.commandAT[6] == 0x00)
                     && (xbee802.commandAT[7] == 0x0F)
                     && (xbee802.commandAT[8] == 0xFF))
            {
              USB.println(F("--> Hardware serie: BRAZIL"));
            }
            else if ((xbee802.commandAT[1] == 0xFF)
                     && (xbee802.commandAT[2] == 0xFF)
                     && (xbee802.commandAT[3] == 0xFF)
                     && (xbee802.commandAT[4] == 0xFE)
                     && (xbee802.commandAT[5] == 0x00)
                     && (xbee802.commandAT[6] == 0x00)
                     && (xbee802.commandAT[7] == 0x00)
                     && (xbee802.commandAT[8] == 0x00))
            {
              USB.println(F("--> Hardware serie: AUSTRALIA"));
            }
          }
          else
          {
            USB.println(F("--> Hardware serie: ERROR"));
          }
        }
        // check for XBee 868LP
        else if (xbee802.commandAT[0] == 8)
        {
          USB.println(F("--> XBee type: 868LP"));
        }

      }
      else if (firmware[0] >= 0x80)
      {
        radio_type = 3; // DigiMesh
        USB.println(F("--> XBee type: DigiMesh"));
      }
      else if ((firmware[0] >= 0x20) && (firmware[1] < 0xB0))
      {
        radio_type = 4; //ZB
        USB.println(F("--> XBee type: ZigBee"));
      }
      else if (firmware[0] == 0x00 && firmware[1] >= 0x02)
      {
        radio_type = 5; // 900 MHz Intl
        USB.println(F("--> XBee type: 900 International"));
      }
      else
      {
        radio_type = 0;
      }

      /////////////////////////////////////////////////////////////
      // Get the XBee MAC address
      /////////////////////////////////////////////////////////////
      if (radio_type != 0)
      {
        xbee802.OFF();
        delay(1000);
        xbee802.ON();
        delay(1000);
        xbee802.flush();

        // Get the XBee MAC address
        int counter = 0;
        while ((xbee802.getOwnMac() != 0) && (counter < 12))
        {
          xbee802.getOwnMac();
          counter++;
        }


        // convert mac address from array to string
        Utils.hex2str(xbee802.sourceMacHigh, macHigh, 4);
        Utils.hex2str(xbee802.sourceMacLow,  macLow,  4);

        // Get the XBee MAC address
        while ((xbee802.getOwnMac() != 0) && (counter < 12))
        {
          xbee802.getOwnMac();
          counter++;
        }

        // convert mac address from array to string
        Utils.hex2str(xbee802.sourceMacHigh, macHigh, 4);
        Utils.hex2str(xbee802.sourceMacLow,  macLow,  4);

        USB.print(F("--> MAC address: "));
        USB.print(macHigh);
        USB.println(macLow);

      }

      if (radio_type < 5)
      {
        USB.print(F("--> Firmware version: "));
        USB.print(firmware[0], HEX);
        USB.println(firmware[1], HEX);
      }

      if (radio_type == 5)
      {
        USB.print(F("--> Firmware version: "));
        USB.printHex(firmware[0]);
        USB.printHex(firmware[1]);
        USB.printHex(firmware[2]);
        USB.printHex(firmware[3]);
        USB.println();
      }
    }
  }


  /////////////////////////////////////////////////////////////
  // 8.2. check for LORAWAN module
  /////////////////////////////////////////////////////////////
  if (radio_type == 0)
  {
    // init object in SOCKET0
    xbee802.OFF();
    uart.setUART(SOCKET0);
    uart.setBaudrate(57600);

    // switch module OFF
    uart.closeUART();
    Utils.setMuxUSB();
    PWR.powerSocket(SOCKET0, LOW);

    delay(500);

    // select multiplexer
    Utils.setMuxSocket0();

    // begin serial communication
    uart.beginUART();

    // power on the socket
    PWR.powerSocket(SOCKET0, HIGH);
    delay(500);
    serialFlush(SOCKET0);

    // check for XBees in SOCKET0
    static char cmd_lorawan[] = "sys get ver\r\n";

    // send command & receive answer
    answer = uart.sendCommand((char*)cmd_lorawan, "RN2483", "RN2903", 1000);

    // send command & receive answer
    uint8_t dev_eui_answer = uart.sendCommand((char*)"mac get deveui\r\n", "\r\n", 1000);

    // check response:
    if (answer == 1)
    {
      USB.println(F("LoRaWAN module is plugged on socket 0:"));
      USB.println(F("--> Hardware serie: LoRaWAN EU"));
      if (dev_eui_answer == 1)
      {
        USB.print(F("--> Device EUI: "));
        USB.print(uart._buffer, uart._length);
      }
      radio_type = 7;
    }
    else if (answer == 2)
    {
      USB.println(F("LoRaWAN module is plugged on socket 0:"));
      USB.println(F("--> Hardware serie: LoRaWAN US"));
      if (dev_eui_answer == 1)
      {
        USB.print(F("--> Device EUI: "));
        USB.print(uart._buffer, uart._length);
      }
      radio_type = 7;
    }
  }



  /////////////////////////////////////////////////////////////
  // 8.4. check for SIGFOX module
  /////////////////////////////////////////////////////////////
  if (radio_type == 0)
  {
    // init object in SOCKET0
    uart.setUART(SOCKET0);
    uart.setBaudrate(9600);

    // switch module OFF
    uart.closeUART();
    Utils.setMuxUSB();
    PWR.powerSocket(SOCKET0, LOW);

    delay(500);

    // select multiplexer
    Utils.setMuxSocket0();

    // begin serial communication
    uart.beginUART();

    // power on the socket
    PWR.powerSocket(SOCKET0, HIGH);
    delay(6000);
    serialFlush(SOCKET0);

    // check for XBees in SOCKET0
    static char cmd_sigfox[] = "AT&V\r";

    // send command & receive answer
    answer = uart.sendCommand((char*)cmd_sigfox, "TD1207", "TD1508", 1000);
    char* sigfox_id;

    // get IMEI
    if (answer != 0)
    {
      // send command & receive answer
      delay(1000);
      response = uart.sendCommand((char*)"ATI7\r", "\r\n", 1000);

      uart. waitFor("\r\n", 1000);
      if (response == 1)
      {
        sigfox_id = strtok( (char*)uart._buffer, "\r\n");
      }
    }

    // check response:
    if (answer == 1)
    {
      USB.println(F("SIGFOX module is plugged on socket 0:"));
      USB.println(F("--> Hardware serie: Sigfox EU"));
      USB.print(F("--> Serial Number: "));
      USB.println(sigfox_id);
      radio_type = 8;
    }
    else if (answer == 2)
    {
      USB.println(F("SIGFOX module is plugged on socket 0:"));
      USB.println(F("--> Hardware serie: Sigfox US"));
      USB.print(F("--> Serial Number: "));
      USB.println(sigfox_id);
      radio_type = 8;
    }
  }




  /////////////////////////////////////////////////////////////
  // 8.5. check for WIFI module
  /////////////////////////////////////////////////////////////
  if (radio_type == 0)
  {
    // init object in SOCKET01
    uart.setUART(SOCKET0);
    uart.setBaudrate(115200);

    // switch module OFF
    uart.closeUART();
    Utils.setMuxUSB();
    PWR.powerSocket(SOCKET0, LOW);

    delay(500);

    // select multiplexer
    Utils.setMuxSocket0();

    // begin serial communication
    uart.beginUART();

    // power on the socket
    PWR.powerSocket(SOCKET0, HIGH);
    delay(3000);
    serialFlush(SOCKET0);

    // check for XBees in SOCKET0
    static char cmd_wifi[] = "AT+i\r";

    // send command & receive answer
    answer = uart.sendCommand((char*)cmd_wifi, "I/OK", 1000);

    // check response:
    if (answer == 1)
    {
      USB.println(F("WIFI PRO module is plugged on socket 0:"));
      radio_type = 9;
    }
  }



  /////////////////////////////////////////////////////////////
  // 8.6. check for 4G module
  /////////////////////////////////////////////////////////////
  if (radio_type == 0)
  {
    // init object in SOCKET1
    uart.setUART(SOCKET1);
    uart.setBaudrate(115200);

    // switch module OFF
    uart.closeUART();
    pinMode(GPRS_PW, OUTPUT);
    digitalWrite(GPRS_PW, LOW);
    delay(500);

    // select multiplexer
    Utils.setMuxSocket1();

    // begin serial communication
    uart.beginUART();

    // Power on the module
    digitalWrite(GPRS_PW, LOW);
    delay(500);
    digitalWrite(GPRS_PW, HIGH);
    delay(11000);
    serialFlush(SOCKET1);

    // send command & receive answer
    answer = uart.sendCommand((char*)"AT#CGMM\r", "LE910-EUG",  "LE910-NAG", "LE910-AU", 1000);
    char* imei;

    // send CPIN command
    uint8_t pin_ready;
    pin_ready = uart.sendCommand((char*)"AT+CPIN?\r", "+CPIN: READY", "ERROR", 1000);


    // get IMEI
    if (answer != 0)
    {
      // send command & receive answer
      response = uart.sendCommand((char*)"AT#CGSN\r", "OK", 1000);

      if (response == 1)
      {
        imei = strtok( (char*)uart._buffer, "AT#CGSN: \r\n");
      }
    }

    // check response:
    if (answer == 1)
    {
      USB.println(F("4G module is plugged on socket 1:"));
      USB.println(F("--> Hardware serie: LE910-EUG (4G EUROPE/BRAZIL)"));
      if (pin_ready == 1) USB.println(F("--> SIM card: OK"));
      else USB.println(F("--> SIM card: ERROR"));
      radio_type = 10;
    }
    else if (answer == 2)
    {
      USB.println(F("4G module is plugged on socket 1:"));
      USB.println(F("--> Hardware serie: LE910-NAG (4G USA)"));
      if (pin_ready == 1) USB.println(F("--> SIM card: OK"));
      else USB.println(F("--> SIM card: ERROR"));
      radio_type = 10;
    }
    else if (answer == 3)
    {
      USB.println(F("4G module is plugged on socket 1:"));
      USB.println(F("--> Hardware serie: LE910-AU (4G AUSTRALIA)"));
      if (pin_ready == 1) USB.println(F("--> SIM card: OK"));
      else USB.println(F("--> SIM card: ERROR"));
      USB.print(F("--> IMEI: "));
      USB.println(imei);
      radio_type = 10;
    }
  }



  /////////////////////////////////////////////////////////////
  // 9. Print module information
  /////////////////////////////////////////////////////////////

  if (radio_type == 0)
  {
    USB.println(F("No radio module detected"));
  }

  USB.println();
  USB.println(F("==============================="));
}





void loop()
{
  // set Green LED
  Utils.setLED(LED1, LED_ON);

  ////////////////////////////////////////////////
  // 1. Waspmote Frame composition
  ////////////////////////////////////////////////

  // Create new frame
  frame.setID(node_id);
  frame.createFrame(ASCII);

  // add low MAC address in the case it is an XBee module
  if ((radio_type < 6) && (radio_type > 0))
  {
    frame.addSensor(SENSOR_MAC, macLow);
  }
  frame.addSensor(SENSOR_ACC, ACC.getX(), ACC.getY(), ACC.getZ());
  frame.addSensor(SENSOR_BAT, PWR.getBatteryLevel());
  frame.showFrame();


  ////////////////////////////////////////////////
  // 2. Send the packet
  ////////////////////////////////////////////////

  if (radio_type == 0)
  {
    USB.println(F("the frame above is printed just by USB (no radio module detected)"));
  }
  else if (radio_type < 6)
  {
    // // In the case of XBee 802.15.4
    if (radio_type == 1)
    {
      // turn XBee on
      xbee802.ON();
      // sets Destination parameters
      xbee_error = xbee802.send(destination, frame.buffer, frame.length);
      // check TX flag
      if (xbee_error == 0)
      {
        USB.println(F("the frame above was sent"));
      }
      else
      {
        USB.println(F("sending error"));
      }
    }
    else
    {
      // In the case of DigiMesh / 868 / 900 / ZigBee
      // turn XBee on
      xbeeZB.ON();
      // sets Destination parameters
      xbee_error = xbeeZB.send(destination, frame.buffer, frame.length);

      // check TX flag
      if (xbee_error == 0)
      {
        USB.println(F("the frame above was sent"));
      }
      else
      {
        USB.println(F("sending error"));
      }
    }
  }



  ///////////////////////////////////////////////////////////////////////////////////////////
  // 3. Deep Sleep
  ///////////////////////////////////////////////////////////////////////////////////////////

  USB.println(F("enter deep sleep"));
  // Go to sleep disconnecting all switches and modules
  // After 10 seconds, Waspmote wakes up thanks to the RTC Alarm
  PWR.deepSleep("00:00:00:05", RTC_OFFSET, RTC_ALM1_MODE1, ALL_OFF);

  USB.ON();
  USB.println(F("\nwake up"));

  // After wake up check interruption source
  if ( intFlag & RTC_INT )
  {
    // clear interruption flag
    intFlag &= ~(RTC_INT);

    USB.println(F("---------------------"));
    USB.println(F("RTC INT captured"));
    USB.println(F("---------------------"));
  }

  RTC.ON();
  ACC.ON();
}


