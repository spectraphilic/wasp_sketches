/*  
 * Script to read file log file on SD card from Waspmote
 * Simon Filhol, simon.filhol@geo.uio.no
 * August 29, 2016
 */

// define file name: MUST be 8.3 SHORT FILE NAME
int numlines2print = 20;
char filename[20];
char lines[10];
int totallines;
int val;


// define variable
uint8_t sd_answer;


void setup()
{
  // open USB port
  USB.ON();
  USB.println("Script to read file on SD card through the Serial");
  USB.println("-------------------------------------------------");
  
  
  // Set SD ON
  SD.ON();
  delay(1000);  
}


void loop()
{ 
  // 1. list file with size and number of lines
  USB.println(F("\n---------------------------"));
  USB.println(F("list recursively showing date and file size (bytes):"));
  USB.println(F("---------------------------"));
  SD.ls( LS_R | LS_DATE | LS_SIZE );   
  USB.println(F("---------------------------"));
  delay(100);
  
  
  // 2. ask for which file to read
  USB.print(F("Enter filename to read: "));
  while(!USB.available()){}
  strcpy( filename, "" );
  while (USB.available() > 0)
  {
    val = USB.read();
    snprintf(filename, sizeof(filename),"%s%c", filename,val);
  }
  USB.println(filename);
  
  if (SD.isFile(filename)==1){
    
      // 3. ask for how many lines to read
      totallines = SD.numln(filename);
      USB.println("------------------");
      USB.print("File ");
      USB.print(filename);
      USB.print(" has ");
      USB.print(totallines);
      USB.println(" lines");
      USB.println("------------------");
      
         
        USB.print(F("Enter how many line to print : "));
        while(!USB.available()){}
        strcpy( lines, "" );
        while (USB.available()) {
          val = USB.read();
          snprintf(lines, sizeof(lines),"%s%c", lines,val);
        }
        USB.flush();
        int linetoprint = atoi(lines);
        USB.println(linetoprint);      
      
      // 4. print to serial the lines
    USB.print("Printing the last ");
    USB.print(linetoprint);
    USB.print(" lines ");
        for(int i = totallines-linetoprint; i < totallines; i++){
      SD.catln(filename, i,1);
      USB.print(SD.buffer);
      USB.flush();
    }

    USB.println(F("-----------------------------")); 
    USB.println(F("*****************************"));
   
    delay(1000); 
    
    
  }
  else{
    USB.print(F("The filename you enter does not exist"));
    return;
  }
}


