
/*

Script to read and change SDI 12 sensor address


*/

#include <SDI12.h>
#include <SDI12.h>

#define DATAPIN 6         // change to the proper pin (JH) 6 = DIGITAL 6 on Waspmote
SDI12 mySDI12(DATAPIN);
char sdiResponse[30];
int i;
/*
  '?' is a wildcard character which asks any and all sensors to respond
  'I' indicates that the command wants information about the sensor
  '!' finishes the command
*/

void setup() {
  USB.ON();
  PWR.setSensorPower(SENS_5V, SENS_ON); // (JH)
  delay(500); // Sensor exitation delay

  mySDI12.begin();

  // -'-'-'-'-'-'-INFO COMMAND-'-'-'-'-'-'-
  mySDI12.sendCommand("?!");
  i = 0;

  // delay(300);                  // wait a while for a response
  while (mySDI12.available())  // write the response to the screen
  {
    char c = mySDI12.read();
    if ((c != '\n') && (c != '\r'))
      {
        sdiResponse[i] = c;
        i++;
      }
      delay(5);
    }
    USB.println(sdiResponse); //write the response to the screen

    delay(1000);
    mySDI12.flush();
  }

  void loop()
  {
//    // -'-'-'-'-MEASUREMENT COMMAND-'-'-'-'-
//    mySDI12.sendCommand("0M!");
//    // delay(300);                   // let the data transfer
//    while (mySDI12.available())   // write the response to the screen
//    {
//      USB.printf("%c", mySDI12.read());
//    }
//    delay(1000); // Needed for CTD10 pressure sensor
//    mySDI12.flush();
//
//
//    // -'-'-'-'-'-'-DATA COMMAND-'-'-'-'-'-'- // (JH) Working examle
//    mySDI12.sendCommand("0D0!");
//
//    // delay(300);                  // let the data transfer
//    while (mySDI12.available())  // write the response to the screen
//    {
//      USB.printf("%c", mySDI12.read());
//    }
//    mySDI12.flush();

    USB.println("- - - - - - - - - - - - - - - -");

    delay(5000); //
  }





























