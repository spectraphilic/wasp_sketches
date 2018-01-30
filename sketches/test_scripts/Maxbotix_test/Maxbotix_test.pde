#include <WaspUIO.h>
#include <WaspFrame.h>
#include <WaspXBeeDM.h>



/*  
 *  ------ MaxBotix (MB7389) test code -------- 
 * John Hulth, 2016-06-22
 */

char filename[]="/data/log.txt";

void setup() 
{

}

void loop() 
{
  PWR.setSensorPower(SENS_3V3,SENS_ON);
  delay(200);
  readMaxbotixSerial();

  USB.println("Done!!!");
  PWR.setSensorPower(SENS_3V3,SENS_OFF);
  delay(5000);
}






///* Function to read the Maxbotix (MB7389) sensor 
// * 
// * Parameters: uint8_t samples  - Number of readings to sample
// *                              - Defult is set to 5
// * 
// * Returns: uint8_t             - median value of samples
// *
// * Wiring: GND -> GND, V+ -> 3V3, pin5 -> AUX SERIAL 1RX
// */
uint8_t readMaxbotixSerial(uint8_t samples)
{
  const int bytes = 4; // Number of bytes to read
  char data_buffer[bytes]; // Store serial data
  int sample; // Store each sample
  

  Utils.setMuxAux1(); // check the manual to find out where you connect the sensor

  PWR.setSensorPower(SENS_3V3,SENS_ON); //  sensor needs 3.3 voltage
  delay(1000);

  beginSerial(9600,1); // set boud rate to 9600

  for (int j = 0; j < samples;)
  {
    // flush and wait for a range reading
    serialFlush(1);

    while (!serialAvailable(1) || serialRead(1) != 'R');

    // read the range
    for (int i = 0; i < bytes; i++) 
    {
      while (!serialAvailable(1));

      data_buffer[i] = serialRead(1);
    }

    sample = (atoi(data_buffer));

    if (sample<=300 || sample>=5000)
    {
      USB.println("NaN"); // For debug information
      delay(10);
    }
    else
    {
      j++;
      USB.println(sample); // add a function for median value....  construct an array...
      delay(1000);
    }
  }
}

// Set defult to 5 samples
uint8_t readMaxbotixSerial(void)
{
  return readMaxbotixSerial(5);
}




