/*  
 */

#include <WaspFrame.h>
#include <WaspUIO.h>

// define the Waspmote ID 
char moteID[] = "";
char path[]="/data";
char filename[]="/data/log.txt";
uint8_t sd_answer;


void setup()
{
  // Init USB port & RTC
  USB.ON();    
  RTC.ON();

  // set the Waspmote ID
  frame.setID(moteID);  


  // Set SD ON
  SD.ON();

  // create path
  sd_answer = SD.mkdir(path);

  if( sd_answer == 1 ) // !!!! 
  { 
    USB.println(F("path created"));
  }
  else
  {
    USB.println(F("mkdir failed"));
  }


  // Create file for Waspmote Frames
  sd_answer = SD.create(filename);

  if( sd_answer == 1 )
  { 
    USB.println(F("/data/log created"));
  }
  else
  {
    USB.println(F("/data/log not created"));
  }
}

void loop()
{

  USB.println(F("Creating an ASCII frame"));

  // Create new frame (ASCII)
  frame.createFrame(ASCII); 

  // set frame fields (Battery sensor - uint8_t)
  frame.addSensor(SENSOR_BAT, (uint8_t) PWR.getBatteryLevel());
  // set frame fields (Temperature in Celsius sensor - float)
  frame.addSensor(SENSOR_IN_TEMP, (float) RTC.getTemperature());

  //frame.addSensor(SENSOR_TIME, (uint8_t) RTC.getTime());

  // Prints frame
  frame.showFrame();


  FrameToSd();



  delay(5000);

  // Wait for five seconds
  delay(1000);

}




/////////////////////////////////////////////////////   
// Append data into file
/////////////////////////////////////////////////////
uint8_t FrameToSd(void)
{
  char path[]="/data";
  char filename[]="/data/temp.txt";

  // define folder and file to store data
  uint8_t sd_answer;

  // open USB port
  USB.ON();

  USB.println(frame.buffer,frame.length);

  sd_answer = SD.appendln(filename, frame.buffer, frame.length);

  if(sd_answer == 1)
  {
    USB.println(F("Frame appended to file"));
  }
  else 
  {
    USB.println(F("Append failed"));
  }
  return sd_answer;
}


/////////////////////////////////////////////////////
//  Get information from File
///////////////////////////////////////////////////// 

uint8_t SDToFrame(void)
{
  char filename[]="/data/log.txt";
  int32_t numLines;
  uint8_t sd_answer;

  // get number of lines in file
  numLines = SD.numln(filename);

  for( int i=numLines-1; i<1 ; i-- )
  {
    // Get line -> SD.buffer
    SD.catln(filename, i, 1); 

    USB.print(F("Get previously stored frame:"));
    USB.println(SD.buffer);
  }
  return sd_answer;
}






























