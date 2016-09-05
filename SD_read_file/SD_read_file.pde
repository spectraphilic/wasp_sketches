/*  
 * Script to read file log file on SD card from Waspmote
 * Simon Filhol, simon.filhol@geo.uio.no
 * August 29, 2016
 */

// define file name: MUST be 8.3 SHORT FILE NAME
char filename[]="loog.TXT";
int numlines2print = 5;

// array in order to read data
char output[101];

// define variable
uint8_t sd_answer;


void setup()
{
  // open USB port
  USB.ON();
  USB.print("read file");
  USB.println(filename);
  
  // Set SD ON
  SD.ON();
  delay(1000);  
}


void loop()
{ 
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


