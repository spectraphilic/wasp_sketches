// Put your libraries here (#include ...)

//#include <WaspFrame.h>
#include <WaspSD.h>
#include <WaspXBeeDM.h>
#include <WaspUIO.h>
#include <WaspUSB.h>

char RX_ADDRESS[] = "";
char WASPMOTE_ID[] = "node_01";

char filename[] = "logfile.txt";

void setup() {
    // put your setup code here, to run once:
    USB.ON();
    RTC.ON();
    SD.ON();
    //xbeeDM.ON();
    USB.println("Board on");
    xbeeDM.ON();
    
    SD.create(filename);
    USB.println("log file created");
    

}


void loop() {
  if(USB.available()){
    USB.print("enter message to write:");
    int m = USB.read();
    char message[20];
    Utils.long2array(m,message);
    USB.println(m);
    
    UIO.initNet('Finse');
    UIO.readMaxbotixSerial();
    USB.println("\n message logged");
  }
  
}

//////////////////////////////////////////////////////////////////////////////////////////
//


