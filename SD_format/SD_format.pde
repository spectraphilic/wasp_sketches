/*  
 *  ------  Format SD file  -------- 
 *By Simon Filhol, Sept 2016
 *
 */

// variable
boolean answer;

void setup()
{
  // Open USB port
  USB.ON();
  USB.println(F("Script to format SD card"));
    
  USB.print(F("\n"
    "This sketch erases and formats SD/SDHC cards.\n"
    "\n"
    "Erase uses the card's fast flash erase command.\n"
    "Flash erase sets all data to 0X00 for most cards\n"
    "and 0XFF for a few vendor's cards.\n"
    "\nWarning, all data on the card will be erased.\n"));

}


void loop()
{
  /* First of all, teh user executes these functions 
  *  when the system is sure. The formatting process
  *  must be done securely
  */
  char c;  
  USB.print(F("Enter 'Y' to format the SD card: "));
  USB.flush();
  while (!USB.available()) {
  }
  delay(400);

  c = USB.read();
  USB.println(c);

  if (c != 'Y') 
  {
    USB.print(F("Quiting, you did not enter 'Y'.\n"));
    return;
  }
  
  // Set SD ON
  SD.ON();

  // 4. format SD card
  USB.print(F("Formatting..."));
  answer = SD.format();  

  if( answer == true ) 
  {
    USB.println(F("SD format OK"));
    USB.println(F("You're DONE!!!!"));
  }
  else
  {
    USB.println(F("SD format failed"));
  }



  USB.println(F("\n\n**************************************************\n\n"));


  // Set SD OFF
  SD.OFF();

  delay(5000);
}

