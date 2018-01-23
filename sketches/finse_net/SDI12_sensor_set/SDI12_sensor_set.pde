/*

  Script to read and change SDI 12 sensor address
  Simon Filhol, August 2017

*/

#include <SDI12.h>


#define DATAPIN DIGITAL8         // change to the proper pin (JH) 6 = DIGITAL 6 on Waspmote
SDI12 mySDI12(DATAPIN);

char sdiResponse[30];
char cmd[10];

char ret_input = 'Y';
char newAddr = '0';
char oldAddr = '?';



int i;
/*
  '?' is a wildcard character which asks any and all sensors to respond
  'I' indicates that the command wants information about the sensor
  '!' finishes the command
*/

void setup() {
  USB.ON();
  PWR.setSensorPower(SENS_5V, SENS_ON);
  delay(500); // Sensor exitation delay

  mySDI12.begin();
  delay(500);
  mySDI12.read_address();


  USB.println("SDI12 sensor information:");

  snprintf( cmd, sizeof(cmd), "%cI!", oldAddr ); // Info command (?I!)
  SDIdata(cmd);


  USB.println("Do you want to change the sensor address?");
  ret_input = input_serial("Y/N: ");


  if (ret_input == 'Y' || ret_input == 'y')
  {

    ret_input = input_serial("Type old sensor address (e.g. 3):");
    oldAddr = ret_input;

    ret_input = input_serial("Type new sensor address (e.g. 3):");
    newAddr = ret_input;
    //oldAddr = mySDI12.sdi12_buffer;

    USB.print("Old sensor address: ");
    USB.println(oldAddr);

    USB.print("New sensor address: ");
    USB.println(newAddr);

    ret_input = input_serial("Apply new address? Y/N: ");

    if (ret_input == 'Y' || ret_input == 'y')
    {
      mySDI12.set_address(oldAddr, newAddr);
      // mySDI12.read_address();
    }
  }
  else
  {
    USB.println("No new address for sensor...");
  }

  USB.println("Adress is:");
  snprintf( cmd, sizeof(cmd), "%cI!", newAddr ); // Info command (?I!)
  SDIdata(cmd);

  USB.println("Done!  \t Restart waspmote");

}

void loop()
{

}

// Function to read input from serial
char input_serial(const char* question) {
  char c;
  USB.print(question);
  USB.flush();
  while (!USB.available()) {
  }
  delay(400);

  c = USB.read();
  USB.println(c);
  return c;
}

// Function to read data from SDI-12 sensor
void SDIdata(char* cmd)
{
  mySDI12.sendCommand(cmd);

  int i = 0;
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
  //SD.appendln(filename, sdiResponse); //write the response to SD-card

  delay(1000); // Needed for CTD10 pressure sensor
  mySDI12.flush();
  memset (sdiResponse, 0, sizeof(sdiResponse));
}

