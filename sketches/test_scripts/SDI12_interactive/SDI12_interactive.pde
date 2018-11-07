
  
  
  
  #include <WaspFrame.h>
  #include <WaspGPS.h>
  #include <SDI12.h>
  
  
  // SDI-12
  SDI12 mySDI12(DIGITAL8);
  char sdiResponse[30];
  int i;
  
  void setup()
  {
    // open USB port
    USB.ON();
    USB.println(F("Waking-up"));
  
  
    PWR.setSensorPower(SENS_5V, SENS_ON); // (JH)
    delay(500); // Sensor exitation delay
  
  
    mySDI12.begin();
    delay(500);
    USB.println(F("Reading sensor address"));
    mySDI12.read_address();
    USB.println(F("end of setup, Going to loop"));
  }
  
  void loop()
  {
    // Loop every 20sec
    delay(10000);
   
  
    // Raedung timestamp from RTC
    USB.print(RTC.getTime());
  
    USB.print(F(","));
  
  
    // Sample data from SDI-12 sensor
    PWR.setSensorPower(SENS_5V, SENS_ON); // (JH)
    delay(500); // Sensor exitation delay
  
    mySDI12.begin();
  
    // -'-'-'-'-MEASUREMENT COMMAND-'-'-'-'-
    mySDI12.sendCommand("1M!");
    SDIdata();
  
  
    // -'-'-'-'-'-'-DATA COMMAND-'-'-'-'-'-'-
    mySDI12.sendCommand("1D0!");
    SDIdata();
  
    USB.println("");
  
  }
  
  
  
  // Function to rad data from SDI-12 sensor
  void SDIdata(void)
  {
    i = 0;
    while (mySDI12.available())  // write the response to the screen
    {
      char c = mySDI12.read();
      if ((c != '\n') && (c != '\r'))
      {
        sdiResponse[i] = c;
        i++;
      }
      delay(5);
    }
    USB.print(sdiResponse); //write the response to the screen
  
    USB.print(F(","));
  
  
    delay(1000); // Needed for CTD10 pressure sensor
    mySDI12.flush();
    memset (sdiResponse, 0, sizeof(sdiResponse));
  }
  
  
  
  

