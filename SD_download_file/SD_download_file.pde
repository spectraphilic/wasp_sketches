/*  
 * Script to download file from Waspmote SD card using python script download_file.py 
 * Simon Filhol, simon.filhol@geo.uio.no
 * August 29, 2016
 */

#include <Serial.h>

// define file name: MUST be 8.3 SHORT FILE NAME
char filename[]="";
int numlines2print = 1;

// array in order to read data
char output[101];

// define variable
uint8_t sd_answer;


void setup()
{
  // open USB port
  USB.ON();
  USB.print("ready for python");  
  // Set SD ON
  SD.ON();
  USB.println(F("List of file available on SD showing date last modified and file size (byte):"));
  SD.ls( LS_R | LS_DATE | LS_SIZE );
  USB.println(F("---------------------------"));
  USB.print();
  delay(1000); 
  USB.print("Filename to download?");
  filename = USB.read();
  delay(2000);

}


void loop()
{ 
  while(USB.available() == 0){}
  if(SD.isFile(filename) == 1){
    int totallines = SD.numln(filename);
    USB.print("File ");
    USB.print(filename);
    USB.println(" exists");
    USB.println("Ready to upload");
    for(int i = 0; i < totallines; i++){
      SD.catln(filename, i,1);
      USB.println( SD.buffer);
      if(i >= totallines)
      {
        USB.read("Finish uploading file");
      }
    }



  }
  break();

}

//==========================================

  if(SD.isFile(filename) == 1){
    int totallines = SD.numln(filename);
    USB.print("Printing last ");
    USB.print(numlines2print);
    USB.print(" lines ");
    SD.catln(filename, totallines-numlines2print,numlines2print);
    USB.println( SD.buffer);
    USB.println(F("-----------------------------")); 
    USB.println(F("*****************************"));
   
    delay(10000); 
  }
  else
  {
    // if the filename is not correct, it shows all the files on the SD card
    USB.print("No File ");
    USB.print(filename);
    USB.println(" existing ");
    USB.println(F("\n---------------------------"));
    USB.println(F("List of file available on SD showing date last modified and file size (byte):"));
    SD.ls( LS_R | LS_DATE | LS_SIZE );
    USB.println(F("---------------------------"));
    delay(10000);
  }
}


