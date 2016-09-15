/*  
 * Script to download file from Waspmote SD card using python script download_file.py 
 * Simon Filhol, simon.filhol@geo.uio.no
 * August 29, 2016
 */


// define file name: MUST be 8.3 SHORT FILE NAME
char filename[20];
int numlines2print = 1;
int val;

// array in order to read data
char output[101];

// define variable
uint8_t sd_answer;


void setup()
{
  // open USB port
  USB.ON();
  USB.println(F("ready for python"));  
  // Set SD ON
  SD.ON();
  USB.println(F("List of file available on SD showing date last modified and file size (byte):"));
  SD.ls( LS_R | LS_DATE | LS_SIZE );
  USB.println(F("---------------------------"));
  delay(1000); 


}


void loop()
{ 
  USB.println(F("============================================="));
  USB.println(F("Filename to download?"));
  while(!USB.available()){}
  strcpy( filename, "" );
  while (USB.available() > 0)
  {
    val = USB.read();
    snprintf(filename, sizeof(filename),"%s%c", filename,val);
  }
  USB.println(F("******"));
  USB.println(filename);

  if(SD.isFile(filename) == 1){
    int totallines = SD.numln(filename);
    USB.print(F("File "));
    USB.print(filename);
    USB.println(F(" exists"));
    USB.println(F("Ready to upload"));
    for(int i = 0; i < totallines; i++){
      SD.catln(filename, i,1);
      USB.print(SD.buffer);
      if(i >= totallines)
      {
        USB.println(F("Finish uploading file"));
      }
    }
  }
  else
  {
    // if the filename is not correct, it shows all the files on the SD card
    USB.print(F("No File "));
    USB.print(filename);
    USB.println(F(" existing "));
    USB.println(F("\n---------------------------"));
    USB.println(F("List of file available on SD showing date last modified and file size (byte):"));
    SD.ls( LS_R | LS_DATE | LS_SIZE );
    USB.println(F("---------------------------"));
  }
}


