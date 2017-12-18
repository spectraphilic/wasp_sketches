
#include <SDI12.h>

#define DATAPIN DIGITAL8         // change to the proper pin (JH) 6 = DIGITAL 6 on Waspmote
SDI12 mySDI12(DATAPIN);
char sdiResponse[30];
char cmd[10];

char sdi_adress[] = "abc"; // SDI-Adresses for the sensors in use, ex. "abc", "12345", "ABCD" or "aB1bC2"

/*
  '?' is a wildcard character which asks any and all sensors to respond
  'I' indicates that the command wants information about the sensor
  '!' finishes the command
*/

void setup()
{
  USB.ON();
  PWR.setSensorPower(SENS_5V, SENS_ON); // (JH)
  delay(500); // Sensor exitation delay

  mySDI12.begin();

  // -'-'-'-'-'-'-INFO COMMAND-'-'-'-'-'-'-
  for (int i = 0; i < (sizeof(sdi_adress) - 1) ; i++)
  {
    snprintf( cmd, sizeof(cmd), "%cI!", sdi_adress[i] ); // Info command (?I!)
    SDIdata(cmd);

    snprintf(cmd, sizeof(cmd), "%cM6!", sdi_adress[i]); // Start measurments command (?M6!) DS-2
    SDIdata(cmd);
  }
  USB.println("- - - - - - - - - - - - - - - -");
}

void loop()
{
  // -'-'-'-'-'-'-DATA COMMAND-'-'-'-'-'-'- //
/*
1R6!   4 Chars   Request from the data logger asking the sensor with address 1 to
                 return the response from a measurement. (A measurement
                 command must be sent prior to sending the data command).
1      1 Char    Sensor Address. Pre-pended on all responses, this allows you to
                 know which sensor on your bus is returning the following
                 information. Continuous measurement command reports the
                 average speed, direction and temperature collected since the last continuous command
+13.05 Variable  Wind speed in m/s
+53    Variable  Wind direction in degrees relative to a northward heading.
+22.3  Variable  Air Temperature in °C
+10.45 Variable  Average meridional (u) Wind Speed in m/s
+7.82  Variable  Average zonal (v) Wind Speed in m/s
+13.05 Variable  Maximum Gust speed in m/s since the last interrogation.
*/

  for (int i = 0; i < (sizeof(sdi_adress) - 1) ; i++)
  {
    snprintf(cmd, sizeof(cmd), "%cR6!", sdi_adress[i]); // Continuous measurments command (?R6!) DS-2
    SDIdata(cmd);
  }

  // USB.println("- - - - - - - - - - - - - - - -");

  delay(10000); // Must be grater then 10sec if using continuous measurments (?R6!)
                // Do not de-power the sensor
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

