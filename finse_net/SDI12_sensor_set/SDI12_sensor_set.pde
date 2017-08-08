
/*

Script to read and change SDI 12 sensor address
Simon Filhol, August 2017

*/

#include <SDI12.h>
#include <WaspUIO.h>

#define DATAPIN 6         // change to the proper pin (JH) 6 = DIGITAL 6 on Waspmote
SDI12 mySDI12(DATAPIN);

char ret_input = 'Y';
char newAddr = '0';
char oldAddr = '0';



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
  mySDI12.read_address();


  USB.println("Do you want to change the sensor address?");
  ret_input = input_serial("Y/N:");


  if(ret_input== 'Y')
  {
    ret_input = input_serial("Type new sensor address (e.g. 3):");

    newAddr = ret_input;
    strncpy(oldAddr, mySDI12.sdi12_buffer, sizeof(mySDI12.sdi12_buffer));

    USB.print("Old sensor address: ");
    USB.println(oldAddr);

    USB.print("New sensor address: ");
    USB.println(newAddr);

    ret_input = input_serial("Apply new address? Y/N");

    if(ret_input== 'Y'){
      mySDI12.set_address(oldAddr, newAddr);
      mySDI12.read_address();
    }
  }else{

  USB.println("No new address for sensor...");

  }

  USB.println("Done!  \t Restart waspmote");

}

void loop()
{

}





char input_serial(const char* question){
  char val;
  long time;
    USB.println(question);
    time = millis();
    while(millis()-time < 10000)
    {
        if (USB.available() > 0)
        {
            val = USB.read();
        }

        // Condition to avoid an overflow (DO NOT REMOVE)
        if (millis() < time)
        {
            time = millis();  
        }
    }
return val;
}




















