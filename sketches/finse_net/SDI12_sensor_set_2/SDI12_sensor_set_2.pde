/*

  Script to read and change SDI 12 sensor address
  Simon Filhol, August 2017

*/

#include <SDI12.h>



#define SENSOR_BOARD 1
#define POWER_BOARD 1


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

#if POWER_BOARD
  pinMode(14, OUTPUT); // 3V3
  pinMode(15, OUTPUT); // 5V
  pinMode(16, OUTPUT); // 12V
  pinMode(17, OUTPUT); // Lead-Acid power
  pinMode(ANALOG5, INPUT); // Lead-Acid data
#endif

#if SENSOR_BOARD
  pinMode(DIGITAL1 , OUTPUT); // Maxbotix power
  pinMode(DIGITAL2 , OUTPUT); // I2C power
  pinMode(DIGITAL5 , OUTPUT); // Onewire power
  pinMode(DIGITAL7 , OUTPUT); // SDI12 power
  pinMode(DIGITAL6, INPUT); // Onewire data
  pinMode(DIGITAL8, INPUT); // SDI12 data
#endif

  // Power On
#if POWER_BOARD
  digitalWrite(16, HIGH); // 12V
  digitalWrite(15, HIGH); // 5V
#else
  PWR.setSensorPower(SENS_5V, SENS_ON);
#endif
#if (SENSOR_BOARD || POWER_BOARD)
  digitalWrite(DIGITAL7, HIGH);
#endif
  delay(1000);

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
    //oldAddr = mySDI12.buffer;

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

