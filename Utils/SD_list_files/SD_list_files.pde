/* 
 *  ------ List files inside the SD card & battery level -------- 
 */

// define variable
uint8_t sd_answer;

void setup()
{
  // open USB port
  USB.ON();
  USB.println(F("Script to list all files"));
  SD.ON();
}


void loop()
{     
  USB.println(F("\n---------------------------"));
  USB.println(F("list recursively showing date and file size (bytes):"));
  USB.println(F("---------------------------"));
  SD.ls( LS_R | LS_DATE | LS_SIZE );   
  USB.println(F("---------------------------"));
  delay(10000);
  
  // Show the remaining battery level
  USB.print(F("Battery Level: "));
  USB.print(PWR.getBatteryLevel(),DEC);
  USB.print(F(" %"));
  
  // Show the battery Volts
  USB.print(F(" | Battery (Volts): "));
  USB.print(PWR.getBatteryVolts());
  USB.println(F(" V"));
  delay(10000);
}



