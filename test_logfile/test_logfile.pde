
// Put your libraries here (#include ...)

#include <WaspFrame.h>
#include <WaspSD.h>
#include <WaspXBeeDM.h>

char RX_ADDRESS[] = "";
char WASPMOTE_ID[] = "node_01";

void setup() {
    // put your setup code here, to run once:
    USB.ON();
    RTC.ON();
    SD.ON();
    //xbeeDM.ON();
    
    

}


void loop() {
  if(USB.available()){
    USB.print("enter message to write:");
    char message = USB.read();
    USB.println(message);
    logActivity(filename,message,RX_ADDRESS, WASPMOTE_ID );
    USB.println("\n message logged");
  }
  
}

//////////////////////////////////////////////////////////////////////////////////////////

uint8_t logActivity( char filename[], char message[], char RX_ADDRESS[], char WASPMOTE_ID[] )
{
  int answer = 0;
  frame.createFrame(ASCII);
  frame.addSensor(SENSOR_STR,"log");
  frame.addSensor(SENSOR_STR,message); //general message
  frame.addSensor(SENSOR_TIME, RTC.getTime() ); 
  frame.showFrame();

  // Make sure a file called filename exists before writing to SD
  if(SD.isFile(filename) == 1)
  { 
    answer = SD.appendln(filename, frame.buffer, frame.length);
    delay(100);
  }
  
  // send frame to network
  if( xbeeDM.available() )
  {
    xbeeDM.send( RX_ADDRESS, frame.buffer, frame.length );
    delay(100);
  }
  return answer;
}

